#include "savefile.h"
#include "util.h"
#include <fstream>
#include <span>

namespace savepatcher {

void SaveFile::validateData(SaveSpan data, std::string_view target) const {
    if (HeaderBNDSection.charsFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

u64 SaveFile::steamId(SaveSpan data) const {
    return SteamIdSection.castInteger<u64>(data);
}

std::vector<u8> SaveFile::loadFile(std::filesystem::path path) const {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::vector<u8> buffer;
    if (!std::filesystem::exists(path))
        throw exception("Path {} does not exist.", util::toAbsolutePath(path));
    if (!file.is_open())
        throw exception("Could not open file '{}'", util::toAbsolutePath(path));

    buffer = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();
    return buffer;
}

void SaveFile::write(SaveSpan data, std::filesystem::path path) const {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!std::filesystem::exists(path))
        throw exception("Path {} does not exist.", util::toAbsolutePath(path));
    if (!file.is_open())
        throw exception("Could not open file '{}'", util::toAbsolutePath(path));

    validateData(data, "Generated data");
    recalculateChecksums(data);
    file.write(reinterpret_cast<const char *>(data.data()), data.size_bytes());
}

const std::vector<Character> SaveFile::parseSlots(SaveSpan data) const {
    std::vector<Character> buffer;
    for (size_t i{}; i < SlotCount; i++)
        buffer.push_back(Character{data, i});

    return buffer;
}

void SaveFile::refreshSlots() {
    auto newSlots{parseSlots(saveData)};
    slots.swap(newSlots);
}

void SaveFile::copySlot(SaveFile &source, size_t sourceSlotIndex, size_t targetSlotIndex) {
    if (targetSlotIndex > SlotCount || sourceSlotIndex > SlotCount)
        throw exception("Invalid slot index while copying character");

    source.slots[sourceSlotIndex].copy(source.saveData, saveData, targetSlotIndex);
    replaceSteamId(source.saveData); // Each slot has the Steam ID hardcoded, optimally we would replace that in Character
    refreshSlots();
}

void SaveFile::copySlot(size_t sourceSlotIndex, size_t targetSlotIndex) {
    return copySlot(*this, sourceSlotIndex, targetSlotIndex);
}

void SaveFile::appendSlot(SaveFile &source, size_t sourceSlotIndex) {
    size_t firstAvailableSlot{SlotCount + 1};
    for (auto &slot : slots)
        if (!slot.active) {
            firstAvailableSlot = slot.slotIndex;
            break;
        }

    if (firstAvailableSlot == SlotCount + 1)
        throw exception("Could not find an unactive slot to append slot {} to", sourceSlotIndex);
    copySlot(source, sourceSlotIndex, firstAvailableSlot);
}

void SaveFile::appendSlot(size_t sourceSlotIndex) {
    return appendSlot(*this, sourceSlotIndex);
};

void SaveFile::renameSlot(size_t slotIndex, std::string_view name) {
    if (slotIndex > SlotCount)
        throw exception("Invalid slot index while renaming character");

    slots[slotIndex].rename(saveData, name);
    refreshSlots();
}

void SaveFile::replaceSteamId(SaveSpan source) const {
    const auto sourceSteamId{SteamIdSection.bytesFrom(source)};
    const auto targetSteamId{SteamIdSection.bytesFrom(saveData)};
    util::replaceAll(saveData, sourceSteamId, targetSteamId);
}

void SaveFile::recalculateChecksums(SaveSpan data) const {
    auto saveHeaderChecksum{util::GenerateMd5(SaveHeaderSection.bytesFrom(data))};
    SaveHeaderChecksumSection.replace(data, saveHeaderChecksum);
    for (auto &slot : slots)
        slot.recalculateSlotChecksum(data);
}

void SaveFile::setSlotActivity(size_t slotIndex, bool active) {
    slots[slotIndex].setActive(saveData, slotIndex, active);
    refreshSlots();
}

void Character::copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const {
    auto targetSlotSection{ParseSlot(SlotOffset, SlotSection.size, targetSlotIndex)};
    auto targetHeaderSection{ParseHeader(SlotHeaderOffset, SlotHeaderSection.size, targetSlotIndex)};

    targetSlotSection.replace(target, SlotSection.bytesFrom(source));
    targetHeaderSection.replace(target, SlotHeaderSection.bytesFrom(source));
    setActive(target, targetSlotIndex, true);
}

void Character::recalculateSlotChecksum(SaveSpan data) const {
    auto hash{util::GenerateMd5(SlotSection.bytesFrom(data))};
    SlotChecksumSection.replace(data, hash);
}

void Character::setActive(SaveSpan data, size_t index, bool value) const {
    ActiveSection.bytesFrom(data)[index] = value;
}

std::string Character::getTimePlayed(SaveSpan data) const {
    return util::SecondsToTimeStamp(SecondsPlayedSection.castInteger<u32>(data));
}

std::string Character::getName(SaveSpan data) const {
    return NameSection.charsFrom(data);
}

void Character::rename(SaveSpan data, std::string_view newName) const {
    // TODO: Theres some instances of the name being missed, presumably in the save header. The easiest fix would be just
    // replacing all occurances of the old name with the new name, but that would break if two characters had the same name.
    NameSection.replace(data, newName);
}

u64 Character::getLevel(SaveSpan data) const {
    return LevelSection.castInteger<u8>(data);
}

bool Character::isActive(SaveSpan data, size_t index) const {
    return static_cast<bool>(ActiveSection.bytesFrom(data)[index]);
}

size_t Character::getSlotIndex() const {
    return slotIndex;
}

}; // namespace savepatcher

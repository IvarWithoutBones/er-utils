#include "savefile.h"
#include "util.h"
#include <fstream>
#include <span>

namespace savepatcher {

void SaveFile::validateData(SaveSpan data, std::string target) {
    if (HeaderBNDSection.charsFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

u64 SaveFile::steamId(SaveSpan data) const {
    return SteamIdSection.castInteger<u64>(data);
}

std::vector<u8> SaveFile::loadFile(std::string filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    std::vector<u8> data;
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    data = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();
    return data;
}

void SaveFile::write(SaveSpan data, std::string filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    validateData(data, "Generated data");
    replaceSteamId(sourceSave, data);
    recalculateChecksums(data);
    file.write(reinterpret_cast<const char *>(data.data()), data.size_bytes());
}

void SaveFile::replaceSteamId(SaveSpan source, SaveSpan target) const {
    const auto sourceSteamId{SteamIdSection.bytesFrom(source)};
    const auto targetSteamId{SteamIdSection.bytesFrom(target)};

    for (size_t i{}; i < target.size_bytes();) {
        auto it{std::search(target.begin() + i + 1, target.end(), sourceSteamId.begin(), sourceSteamId.end())};
        if (it == target.end())
            break;

        i = it - target.begin();
        const Section section{i, SteamIdSection.size};
        section.replace(target, targetSteamId);
    };
}

void SaveFile::recalculateChecksums(SaveSpan data) const {
    auto saveHeaderChecksum{GenerateMd5(SaveHeaderSection.bytesFrom(data))};
    SaveHeaderChecksumSection.replace(data, saveHeaderChecksum);
    for (auto &character : sourceCharacters)
        character.recalculateSlotChecksum(data);
}

std::vector<Character> SaveFile::parseSlots(SaveSpan data) const {
    std::vector<Character> buffer;
    for (size_t i{}; i < SlotCount; i++)
        buffer.push_back(Character{data, i});

    return buffer;
}

void SaveFile::copyCharacter(size_t sourceSlotIndex, size_t targetSlotIndex) const {
    if (targetSlotIndex >= SlotCount || sourceSlotIndex >= SlotCount || targetSlotIndex < 0 || sourceSlotIndex < 0)
        throw exception("Invalid slot index while copying character");

    sourceCharacters[sourceSlotIndex].copy(sourceSave, targetSave, targetSlotIndex);
}

void SaveFile::appendSlot(size_t sourceSlotIndex) const {
    size_t firstAvailableSlot{SlotCount + 1};
    std::for_each(targetCharacters.rbegin(), targetCharacters.rend(), [&](const Character &slot) {
        if (!slot.active)
            firstAvailableSlot = slot.getSlotIndex();
    });

    if (firstAvailableSlot == SlotCount + 1)
        throw exception("Could not find an unactive slot to append slot {} to", sourceSlotIndex);

    copyCharacter(sourceSlotIndex, firstAvailableSlot);
}

void Character::copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const {
    auto targetSlotSection{ParseSlot(SlotOffset, SlotSection.size, targetSlotIndex)};
    auto targetHeaderSection{ParseHeader(SlotHeaderOffset, SlotHeaderSection.size, targetSlotIndex)};

    targetSlotSection.replace(target, SlotSection.bytesFrom(source));
    targetHeaderSection.replace(target, SlotHeaderSection.bytesFrom(source));
    setActive(target, targetSlotIndex, true);
}

void Character::recalculateSlotChecksum(SaveSpan data) const {
    auto hash{GenerateMd5(SlotSection.bytesFrom(data))};
    SlotChecksumSection.replace(data, hash);
}

void Character::setActive(SaveSpan data, size_t index, bool active) const {
    ActiveSection.bytesFrom(data)[index] = active ? true : false;
}

std::string Character::getTimePlayed(SaveSpan data) const {
    return SecondsToTimeStamp(SecondsPlayedSection.castInteger<u32>(data));
}

std::string Character::getName(SaveSpan data) const {
    return NameSection.charsFrom(data);
};

u64 Character::getLevel(SaveSpan data) const {
    return LevelSection.castInteger<u8>(data);
}

bool Character::isActive(SaveSpan data, size_t slotIndex) const {
    return static_cast<bool>(ActiveSection.bytesFrom(data)[slotIndex]);
}

size_t Character::getSlotIndex() const {
    return slotIndex;
}

}; // namespace savepatcher

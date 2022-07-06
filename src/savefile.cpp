#include "savefile.h"
#include "util.h"
#include <fstream>
#include <span>
#include <string_view>

namespace savepatcher {

void SaveFile::validateData(SaveSpan data, std::string_view target) const {
    if (HeaderBNDSection.charsFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

u64 SaveFile::steamId() const {
    return SteamIdSection.castInteger<u64>(saveData);
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
    replaceSteamId(source.saveData, steamId());
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
    setSlotActivity(firstAvailableSlot, true);
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

void SaveFile::replaceSteamId(SaveSpan replaceFrom, u64 newSteamId) const {
    std::array<u8, sizeof(u64)> steamIdData{};
    std::memcpy(steamIdData.data(), &newSteamId, sizeof(u64));
    util::replaceAll<u8>(saveData, SteamIdSection.bytesFrom(replaceFrom), steamIdData);
}

void SaveFile::replaceSteamId(u64 newSteamId) const {
    replaceSteamId(saveData, newSteamId);
};

void SaveFile::recalculateChecksums(SaveSpan data) const {
    auto saveHeaderChecksum{util::generateMd5(SaveHeaderSection.bytesFrom(data))};
    SaveHeaderChecksumSection.replace(data, saveHeaderChecksum);
    for (auto &slot : slots)
        slot.recalculateSlotChecksum(data);
}

void SaveFile::setSlotActivity(size_t slotIndex, bool active) {
    slots[slotIndex].setActive(saveData, active);
    refreshSlots();
}

u32 SaveFile::getItem(size_t slot, Item item) const {
    return slots[slot].getItem(saveData, item);
}

void SaveFile::setItem(size_t slot, Item item, u32 quantity) const {
    slots[slot].setItem(saveData, item, quantity);
}

void Character::copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const {
    auto targetSlotSection{ParseSlot(SlotOffset, SlotSection.size, targetSlotIndex)};
    auto targetHeaderSection{ParseHeader(SlotHeaderOffset, SlotHeaderSection.size, targetSlotIndex)};
    targetSlotSection.replace(target, SlotSection.bytesFrom(source));
    targetHeaderSection.replace(target, SlotHeaderSection.bytesFrom(source));
    setActive(target, true);
}

void Character::recalculateSlotChecksum(SaveSpan data) const {
    auto hash{util::generateMd5(SlotSection.bytesFrom(data))};
    SlotChecksumSection.replace(data, hash);
}

u32 Character::getItem(SaveSpan data, Item item) const {
    auto slot{SlotSection.bytesFrom(data)};
    const auto itr{std::search(slot.begin(), slot.end(), item.data.begin(), item.data.end())};
    return (itr != slot.end()) ? slot[itr - slot.begin() + item.data.size()] : 0;
}

void Character::setItem(SaveSpan data, Item item, u32 quantity) const {
    size_t quantityOffset{};
    auto slot{SlotSection.bytesFrom(data)};

    if (auto itr = std::search(slot.begin(), slot.end(), item.data.begin(), item.data.end()); itr != slot.end())
        // If the item is already present we can just update the quantity
        quantityOffset = (itr - slot.begin()) + item.data.size();
    else {
        constexpr static auto itemSize{10};

        // If the item isnt already present we need to insert it. This currently works, but for some reason only sometimes
        for (size_t i{}; i < slot.size(); i++) {
            // Check if an item exists at the current position
            if (slot[i] == item.delimiter.front() && slot[i + 1] == item.delimiter.back()) {
                const auto nextItem{i + itemSize};
                const auto nextItemEnd{i + (itemSize * 2)};
                // Check if the 10 bytes after the found item are empty
                if (std::search_n(slot.begin() + nextItem, slot.begin() + nextItemEnd, itemSize, 0x0) != slot.begin() + nextItemEnd) {
                    // If yes, copy the requested item into it
                    std::copy(item.data.begin(), item.data.end(), slot.begin() + nextItem);
                    quantityOffset = nextItem + item.data.size();
                    break;
                } else
                    i += itemSize;
            }
        }
    }

    slot[quantityOffset] = static_cast<u8>(quantity);
}

void Character::rename(SaveSpan data, std::string_view newName) const {
    std::array<u8, NameSection.size> convertedName{};
    util::utf8ToUtf16(convertedName, std::u16string(newName.begin(), newName.end()));
    // Any characters sharing the same name will get replaced with the new name as of now
    util::replaceAll<u8>(data, parsedNameSection.bytesFrom(data), convertedName);
}

void Character::setActive(SaveSpan data, bool value) const {
    ActiveSection.bytesFrom(data)[slotIndex] = value;
}

std::string Character::getTimePlayed(SaveSpan data) const {
    return util::secondsToTimeStamp(SecondsPlayedSection.castInteger<u32>(data));
}

std::string Character::getName(SaveSpan data) const {
    return parsedNameSection.charsFrom(data);
}

u64 Character::getLevel(SaveSpan data) const {
    return LevelSection.castInteger<u8>(data);
}

bool Character::isActive(SaveSpan data, size_t index) const {
    return static_cast<bool>(ActiveSection.bytesFrom(data)[index]);
}

}; // namespace savepatcher

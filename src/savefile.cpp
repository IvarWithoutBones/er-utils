#include "savefile.h"
#include "util.h"
#include <fstream>
#include <span>
#include <string_view>

namespace savepatcher {

void Slot::copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const {
    const auto targetSlotSection{ParseSlot(SlotSectionOffset, SlotSection.size, targetSlotIndex)};
    const auto targetHeaderSection{ParseHeader(SlotHeaderSectionOffset, SlotHeaderSection.size, targetSlotIndex)};
    targetSlotSection.replace(target, SlotSection.bytesFrom(source));
    targetHeaderSection.replace(target, SlotHeaderSection.bytesFrom(source));
}

void Slot::recalculateSlotChecksum(SaveSpan data) const {
    auto hash{GenerateMd5(SlotSection.bytesFrom(data))};
    SlotChecksumSection.replace(data, hash);
}

void Slot::rename(SaveSpan data, std::string_view newName) const {
    std::array<u8, NameSectionSize> convertedName{};
    Utf8ToUtf16(convertedName, std::u16string(newName.begin(), newName.end()));
    // Any characters sharing the same name will get replaced with the new name as of now
    ReplaceAll<u8>(data, NameSection.bytesFrom(data), convertedName);
}

u32 Slot::getItem(SaveSpan data, Item item) const {
    auto slot{SlotSection.bytesFrom(data)};
    const auto itr{std::search(slot.begin(), slot.end(), item.data.begin(), item.data.end())};
    return (itr != slot.end()) ? slot[itr - slot.begin() + item.data.size()] : 0;
}

void Slot::setItem(SaveSpan data, Item item, u32 quantity) const {
    constexpr static auto itemSize{10};
    auto slot{SlotSection.bytesFrom(data)};
    size_t quantityOffset{};

    if (auto itr = std::search(slot.begin(), slot.end(), item.data.begin(), item.data.end()); itr != slot.end())
        // If the item is already present we can just update the quantity
        quantityOffset = (itr - slot.begin()) + item.data.size();
    else {
        // Otherwise we need to insert it. This currently works, but only for a few items.
        for (size_t i{}; i < slot.size(); i++) {
            // Check if an item exists at the current position
            if (slot[i] == item.delimiter.front() && slot[i + 1] == item.delimiter.back()) {
                const auto nextItem{i + itemSize};
                const auto nextItemEnd{i + (itemSize * 2)};
                // Check if the space after the found item is empty
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

    if (!quantityOffset)
        throw exception("Could not find a place for item with quantity {}", quantity);
    slot[quantityOffset] = static_cast<u8>(quantity);
}

std::string Slot::getName(SaveSpan data) const {
    return Utf16ToUtf8String(NameSection.bytesFrom(data));
}

void Slot::setActive(SaveSpan data, bool value) const {
    ActiveSection.bytesFrom(data)[slotIndex] = value;
}

std::string Slot::getTimePlayed(SaveSpan data) const {
    return SecondsToTimeStamp(SecondsPlayedSection.castInteger<u32>(data));
}

u64 Slot::getLevel(SaveSpan data) const {
    return LevelSection.castInteger<u8>(data);
}

bool Slot::isActive(SaveSpan data, size_t index) const {
    return static_cast<bool>(ActiveSection.bytesFrom(data)[index]);
}

void SaveFile::validateData(SaveSpan data, std::string_view target) const {
    if (HeaderBNDSection.stringFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

u64 SaveFile::steamId() const {
    return SteamIdSection.castInteger<u64>(saveData);
}

std::vector<u8> SaveFile::loadFile(std::filesystem::path path) const {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::vector<u8> buffer;
    if (!std::filesystem::exists(path))
        throw exception("Path {} does not exist.", ToAbsolutePath(path).generic_string());
    if (!file.is_open())
        throw exception("Could not open file '{}'", ToAbsolutePath(path).generic_string());

    buffer = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();
    return buffer;
}

void SaveFile::write(SaveSpan data, std::filesystem::path path) const {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!std::filesystem::exists(path))
        throw exception("Path {} does not exist.", ToAbsolutePath(path).generic_string());
    if (!file.is_open())
        throw exception("Could not open file '{}'", ToAbsolutePath(path).generic_string());

    validateData(data, "Generated data");
    recalculateChecksums(data);
    file.write(reinterpret_cast<const char *>(data.data()), data.size_bytes());
}

const std::vector<Slot> SaveFile::parseSlots(SaveSpan data) const {
    std::vector<Slot> buffer;
    for (size_t i{}; i < SlotCount; i++)
        buffer.push_back(Slot{data, i});

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
    setSlotActivity(targetSlotIndex, true);
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
}

void SaveFile::renameSlot(size_t slotIndex, std::string_view name) {
    if (slotIndex > SlotCount)
        throw exception("Invalid slot index while renaming character");

    slots[slotIndex].rename(saveData, name);
    refreshSlots();
}

void SaveFile::replaceSteamId(SaveSpan replaceFrom, u64 newSteamId) const {
    std::array<u8, sizeof(u64)> steamIdData{};
    std::memcpy(steamIdData.data(), &newSteamId, sizeof(u64));
    ReplaceAll<u8>(saveData, SteamIdSection.bytesFrom(replaceFrom), steamIdData);
}

void SaveFile::replaceSteamId(u64 newSteamId) const {
    replaceSteamId(saveData, newSteamId);
}

void SaveFile::recalculateChecksums(SaveSpan data) const {
    auto saveHeaderChecksum{GenerateMd5(SaveHeaderSection.bytesFrom(data))};
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

}; // namespace savepatcher

#include "items.h"
#include "util.h"
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace savepatcher {

/**
 * @brief One of the slots in a save file
 */
class Character {
  public:
    const size_t slotIndex; //!< The index of the save slot, each character has a unique slot. This value can range between 0-9
  private:
    /**
     * @brief Used to calculate a target address when copying a character
     */
    constexpr static const size_t SlotOffset{0x310};
    constexpr static const size_t SlotHeaderOffset{0x1901D0E};

    constexpr static Section ActiveSection{0x1901D04, 0xA};                              //!< Contains booleans indicating if the character at address + slotIndex is active
    constexpr static Section NameSection{0x1901D0E, 0x22};                               //!< Contains the name of a character, without slot index parsing
    const Section SlotSection{ParseSlot(SlotOffset, 0x280000)};                          //!< Contains the save data of the character
    const Section SlotChecksumSection{ParseSlot(0x300, 0x10)};                           //!< Contains the checksum of the data section
    const Section SlotHeaderSection{ParseHeader(SlotHeaderOffset, 0x24C)};               //!< Contains the slots header
    const Section parsedNameSection{ParseHeader(NameSection.address, NameSection.size)}; //!< Contains the name of the character in the current slot
    const Section LevelSection{ParseHeader(0x1901D30, 0x1)};                             //!< Contains the level of the character
    const Section SecondsPlayedSection{ParseHeader(0x1901D34, 0x4)};                     //!< Contains the number of seconds played

    /**
     * @brief A wrapper around Section that provides the offsets for a save header
     */
    constexpr Section ParseHeader(size_t address, size_t size, size_t index) const {
        return Section{address + (index * SlotHeaderSection.size), size};
    }

    constexpr Section ParseHeader(size_t address, size_t size) const {
        return ParseHeader(address, size, slotIndex);
    }

    /**
     * @brief A wrapper around Section that provides the offsets for a save slot
     */
    constexpr Section ParseSlot(size_t address, size_t size, size_t index) const {
        return Section{address + (index * 0x10) + (index * SlotSection.size), size};
    }

    constexpr Section ParseSlot(size_t address, size_t size) const {
        return ParseSlot(address, size, slotIndex);
    }

    bool isActive(SaveSpan data, size_t slotIndex) const;

    std::string getName(SaveSpan data) const;

    std::string getTimePlayed(SaveSpan data) const;

    u64 getLevel(SaveSpan data) const;

  public:
    bool active;            //!< Whether the save slot is currently in use
    u64 level;              //!< The level of the character
    std::string name;       //!< The name of the character
    std::string timePlayed; //!< A timestamp of the characters play time

    constexpr Character(SaveSpan data, size_t slotIndex) : slotIndex{slotIndex}, active{isActive(data, slotIndex)}, level{getLevel(data)}, name{getName(data)}, timePlayed{getTimePlayed(data)} {}

    /**
     * @brief Copy the currently active save slot into the given span
     * @param source The span to copy the save slot from
     * @param target The span to copy the save slot to
     * @param targetSlotIndex The index of the source save slot to copy
     */
    void copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const;

    /**
     * @brief Recalculate and replace the checksum of the currently active save slot
     */
    void recalculateSlotChecksum(SaveSpan data) const;

    /**
     * @brief Get the quanity of an item
     */
    u32 getItem(SaveSpan data, Item item) const;

    /**
     * @brief Set the quantity of an item
     */
    void setItem(SaveSpan data, Item item, u32 quantity) const;

    void setActive(SaveSpan data, bool active) const;

    void rename(SaveSpan data, std::string_view newName) const;
};

/**
 * @brief Elden Ring save file parser and patcher
 */
class SaveFile {
  private:
    constexpr static size_t SlotCount{10}; //!< The number of slots in each save file starting from 0
    std::vector<u8> saveDataContainer;
    SaveSpan saveData;

    constexpr static Section HeaderBNDSection{0x0, 0x3};                 //!< Contains the characters BND, used for validation
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};      //!< Contains the save header
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10}; //!< Contains the MD5 sum of the save header
    constexpr static Section SteamIdSection{0x19003B4, 0x8};             //!< Contains one instance the Steam ID

    std::vector<u8> loadFile(std::filesystem::path path) const;

    /**
     * @brief Replace the Steam ID, recalculate checksums and write the resulting span to a file
     */
    void write(SaveSpan data, std::filesystem::path path) const;

    /**
     * @brief Validate a file is an Elden Ring save file
     * @param target The name of the file to log if validation fails
     */
    void validateData(SaveSpan data, std::string_view target) const;

    /**
     * @brief Recalculate and replace the save header and slot checksums
     */
    void recalculateChecksums(SaveSpan data) const;

    /**
     * @brief Replace the Steam ID inside the target save file
     * @param replaceFrom The data containing the Steam ID to replace
     */
    void replaceSteamId(SaveSpan replaceFrom, u64 newSteamId) const;

    const std::vector<Character> parseSlots(SaveSpan data) const;

    void refreshSlots();

  public:
    std::vector<Character> slots; //!< The characters in the save file

    SaveFile(std::filesystem::path path) : saveDataContainer{loadFile(path)}, saveData{saveDataContainer}, slots{parseSlots(saveData)} {
        validateData(saveData, util::toAbsolutePath(path));
    }

    /**
     * @brief Write the patched save data to a file
     */
    void write(std::filesystem::path path) {
        write(saveData, path);
    }

    /**
     * @brief Copy a character from a source save file
     * @param source The save file to copy from
     * @param sourceSlotIndex The index of the source save slot to copy from
     * @param targetSlotIndex The index of the target save slot to copy to
     */
    void copySlot(SaveFile &source, size_t sourceSlotIndex, size_t targetSlotIndex);

    void copySlot(size_t sourceSlotIndex, size_t targetSlotIndex);

    /**
     * @brief Append a slot from a source save file
     * @param source The save file to copy from
     * @param sourceSlotIndex The index of the source slot to copy
     */
    void appendSlot(SaveFile &source, size_t sourceSlotIndex);

    void appendSlot(size_t sourceSlotIndex);

    /**
     * @brief Rename a character in the given slot
     */
    void renameSlot(size_t slotIndex, std::string_view newName);

    /**
     * @brief Set a slots active state
     */
    void setSlotActivity(size_t slotIndex, bool active);

    /**
     * @brief Get the Steam ID from the save header
     */
    u64 steamId() const;

    /**
     * @brief Replace all occurances of the Steam ID
     */
    void replaceSteamId(u64 newSteamId) const;

    /**
     * @brief Get the quantity of an item in the given slot
     */
    u32 getItem(size_t slot, Item item) const;

    /**
     * @brief Set the quantity of an item in the given slot
     */
    void setItem(size_t slot, Item item, u32 quantity) const;
};

} // namespace savepatcher

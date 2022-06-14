#include "util.h"
#include <span>
#include <string>
#include <vector>

namespace savepatcher {

/**
 * @brief One of the characters in a save file
 */
class Character {
  private:
    const size_t slotIndex;                                          //!< The index of the save slot, each character has a unique slot. This value can range between 0-10.
    constexpr static Section ActiveSection{0x1901D04, 0xA};          //!< Contains booleans indicating if the character at address + slotIndex is active
    const Section SlotSection{ParseSlot(0x310, 0x280000)};           //!< Contains the save data of the character
    const Section SlotChecksumSection{ParseSlot(0x300, 0x10)};       //!< Contains the checksum of the data section
    const Section SlotHeaderSection{ParseHeader(0x1901D0E, 0x24C)};  //!< Contains the slot header
    const Section NameSection{ParseHeader(0x1901D0E, 0x22)};         //!< Contains the name of the character
    const Section LevelSection{ParseHeader(0x1901D30, 0x1)};         //!< Contains the level of the character
    const Section SecondsPlayedSection{ParseHeader(0x1901D34, 0x4)}; //!< Contains the number of seconds played

    /**
     * @brief A wrapper around Section that provides the offsets for a save header
     */
    constexpr Section ParseHeader(size_t address, size_t size) const {
        return Section{address + (slotIndex * SlotHeaderSection.size), size};
    }

    /**
     * @brief A wrapper around Section that provides the offsets for a save slot
     */
    constexpr Section ParseSlot(size_t address, size_t size) const {
        return Section{address + (slotIndex * 0x10) + (slotIndex * SlotSection.size), size};
    }

    std::string getName(SaveSpan data) const;
    std::string getTimePlayed(SaveSpan data) const;
    u64 getLevel(SaveSpan data) const;
    bool isActive(SaveSpan data, size_t slotIndex) const;

  public:
    bool active;            //!< Whether the save slot is currently in use
    u64 level;              //!< The level of the character
    std::string name;       //!< The name of the character
    std::string timePlayed; //!< A timestamp of the characters play time

    constexpr Character(SaveSpan data, size_t slotIndex) : slotIndex{slotIndex}, active{isActive(data, slotIndex)}, name{getName(data)}, level{getLevel(data)}, timePlayed{getTimePlayed(data)} {}

    /**
     * @brief Copy the currently active save slot into the given span
     */
    void copy(SaveSpan source, SaveSpan target, size_t targetSlotIndex) const;

    /**
     * @brief Recalculate and replace the checksum of the currently active save slot
     */
    void replaceSlotChecksum(SaveSpan data) const;
};

/**
 * @brief Elden Ring save file parser and patcher
 * @param filename The path to the save file
 */
class SaveFile {
  private:
    constexpr static size_t SlotCount = 10; //!< The number of characters in the save file
    std::vector<u8> sourceSaveDataContainer;
    std::vector<u8> targetSaveDataContainer;
    SaveSpan sourceSave;
    SaveSpan targetSave;

    constexpr static Section HeaderBNDSection{0x0, 0x3};                 //!< Contains the characters BND, used for validation
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};      //!< Contains the save header
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10}; //!< Contains the MD5 sum of the save header
    constexpr static Section SteamIdSection{0x19003B4, 0x8};             //!< Contains one instance the Steam ID

    std::vector<u8> loadFile(std::string filename);
    void write(SaveSpan data, std::string filename);

    /**
     * @brief Validate a file is an Elden Ring save file
     * @param data The data to validate
     * @param target The name of the file to log if validation fails
     */
    void validateData(SaveSpan data, std::string target);

    /**
     * @brief Load the characters into the characters vector
     */
    std::vector<Character> parseCharacters(SaveSpan data) const;

    /**
     * @brief Recalculate and replace the save header and slot checksums
     */
    void recalculateChecksums(SaveSpan data) const;

    /**
     * @brief Replace the Steam ID inside the target save file
     * @param source The data containing the old Steam ID to replace
     * @param target The data containing the new Steam ID to replace with. The new data gets written to this
     */
    void replaceSteamId(SaveSpan source, SaveSpan target) const;

  public:
    std::vector<Character> characters; //!< The characters in the save file

    constexpr SaveFile(std::string_view sourceFile, std::string_view targetFile) : sourceSaveDataContainer{loadFile(sourceFile.data())}, targetSaveDataContainer{loadFile(targetFile.data())}, sourceSave{sourceSaveDataContainer}, targetSave{targetSaveDataContainer}, characters{parseCharacters(sourceSave)} {
        validateData(sourceSave, sourceFile.data());
        validateData(targetSave, sourceFile.data());
    }

    /**
     * @brief Write the patched save data to a file
     */
    void write(std::string filename) {
        write(targetSave, filename);
    }

    /**
     * @brief Copy a character between the source and target save files
     * @param sourceSlotIndex The index of the source save slot to copy from
     * @param targetSlotIndex The index of the target save slot to copy to
     */
    void copyCharacter(size_t sourceSlotIndex, size_t targetSlotIndex) const;

    /**
     * @brief Get the Steam ID inside of the save header
     */
    u64 steamId(SaveSpan data) const;
    u64 steamId() const {
        return steamId(sourceSave);
    };

    /**
     * @brief Get the checksum of the save header
     */
    std::string checksum(SaveSpan data) const;
    std::string checksum() const {
        return checksum(sourceSave);
    };
};

} // namespace savepatcher

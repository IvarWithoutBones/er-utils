#include "util.h"
#include <span>
#include <string>
#include <vector>

namespace savepatcher {

class Character {
  private:
    size_t slotIndex; //!< The index of the save slot, each character has a index.

    constexpr static size_t SlotHeaderLength = 0x24C;                 //!< The length of a save slot
    constexpr static Section ActiveSection{0x1901D04, 0xA};           //!< The section containing 10 bools indicating if a save slot at the same offset is active
    const Section NameSection{ParseSection(0x1901D0E, 0x22)};         //!< The section containing the name of the character
    const Section LevelSection{ParseSection(0x1901D30, 0x1)};         //!< The section containing the level of the character
    const Section SecondsPlayedSection{ParseSection(0x1901D34, 0x4)}; //!< The section containing the number of seconds played
    const Section DataSection{0x310 + (slotIndex * 0x10) + (slotIndex * 0x280000), 0x280000};

    /**
     * @brief A section of a save slot containing a character
     * @param offset The start index of the section
     * @param size The length of the section
     * @param offset An offset to apply to the address, this defaults to 0
     * @return A section of a save slot
     */
    constexpr Section ParseSection(size_t address, size_t size, size_t offset = 0) const {
        return Section{address + offset + (slotIndex * SlotHeaderLength), size};
    }

    std::string nameFrom(std::span<u8, SaveFileSize> data) const;
    std::string timePlayedFrom(std::span<u8, SaveFileSize> data) const;
    u64 levelFrom(std::span<u8, SaveFileSize> data) const;
    bool activeFrom(std::span<u8, SaveFileSize> data, size_t slotIndex) const;

  public:
    constexpr Character(std::span<u8, SaveFileSize> file, size_t slotIndex) : slotIndex{slotIndex}, active{activeFrom(file, slotIndex)}, name{nameFrom(file)}, level{levelFrom(file)}, timePlayed{timePlayedFrom(file)} {}

    bool active;            //!< Whether the save slot is currently in use
    u64 level;              //!< The level of the character
    std::string name;       //!< The name of the character
    std::string timePlayed; //!< The time played of the character
};

/**
 * @brief Elden Ring save file parser and patcher
 * @param filename The path to the save file
 */
class SaveFile {
  private:
    std::vector<u8> saveDataContainer;         //!< The container the save data is stored in
    std::vector<u8> patchedSaveData;           //!< The save data to modify
    std::span<u8, SaveFileSize> saveData;      //!< A span of the original save data
    std::vector<Character> characterContainer; //!< The characters in the save file

    constexpr static Section HeaderBNDSection{0x0, 0x3};                 //!< The section containing the characters BND, used for validation
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};      //!< The section containing the save header
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10}; //!< The section containing the checksum of the save header
    constexpr static Section SteamIdSection{0x19003B4, 0x8};             //!< The section containing the Steam ID

    std::vector<u8> loadFile(const std::string &filename);

    void validateData(std::span<u8> data, const std::string &target);

    size_t getActiveSlotIndex(std::span<u8> data) const;

    /**
     * @brief Write the patched save data to a file
     * @param filename The path to the save file
     */
    void write(std::span<u8> data, const std::string &filename);

    void copyCharacter(std::span<u8> source, std::span<u8> target, size_t charIndex);

    std::vector<Character> parseCharacters(std::span<u8, SaveFileSize> data) const;

  public:
    SaveFile(const std::string &filename) : saveDataContainer{loadFile(filename)}, saveData{saveDataContainer}, patchedSaveData{saveDataContainer}, characterContainer{parseCharacters(saveData)} {
        validateData(saveData, filename);
        // auto targetSave{loadFile("../backup/ER0000.backup1")};
        // copyCharacter(saveDataContainer, targetSave, 0);
    }

    void write(const std::string &filename) {
        write(patchedSaveData, filename);
    }

    std::vector<Character> characters() {
        return characterContainer;
    }

    /**
     * @brief Replace the Steam ID hardcoded in the save header
     * @param steamId The new Steam ID
     */
    void replaceSteamId(u64 steamId);

    /**
     * @brief Recalculate and replace the checksum of the save header
     */
    std::string recalculateChecksum();

    /**
     * @brief Get the Steam ID inside of the save header
     * @return The Steam ID
     */
    u64 steamId(const std::span<u8> data) const;
    u64 steamId() const {
        return steamId(saveData);
    };

    /**
     * @brief Get the checksum of the save header
     * @return The checksum of the save header
     */
    std::string checksum(const std::span<u8> data) const;
    std::string checksum() const {
        return checksum(saveData);
    };
};

} // namespace savepatcher

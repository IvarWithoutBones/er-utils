#include "util.h"
#include <span>
#include <string>
#include <vector>

namespace savepatcher {

/**
 * @brief Elden Ring save file parser and patcher
 * @param filename The path to the save file
 */
class SaveFile {
  private:
    std::vector<u8> saveDataContainer; //!< The container the save data is stored in
    std::vector<u8> patchedSaveData;   //!< The save data to modify
    std::span<u8> saveData;            //!< A span of the original save data
    size_t activeSlotIndex{};          //!< The index of the active slot in the save file

    /**
     * @brief Completely static data sections
     */
    constexpr static size_t SaveFileSize = 0x1BA03D0;                    //!< The size of the save file, used for validation
    constexpr static size_t SlotLength = 0x24C;                          //!< The length of a save slot
    constexpr static Section HeaderBNDSection{0x0, 0x3};                 //!< The section containing the characters BND, used for validation
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};      //!< The section containing the save header
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10}; //!< The section containing the checksum of the save header
    constexpr static Section SteamIdSection{0x19003B4, 0x8};             //!< The section containing the Steam ID
    constexpr static Section ActiveSection{0x1901D04, 0xA};              //!< The section containing a bool wether the save slot is active

    /**
     * @brief Data sections dependant on the active slot
     */
    const SlotSection NameSection{0x1901D0E, 0x22, activeSlotIndex};         //!< The section containing the name of the character
    const SlotSection LevelSection{0x1901D30, 0x1, activeSlotIndex};         //!< The section containing the level of the character
    const SlotSection SecondsPlayedSection{0x1901D34, 0x4, activeSlotIndex}; //!< The section containing the amount of seconds played

    std::vector<u8> loadFile(const std::string &filename);

    void validateData(std::span<u8> data, const std::string &target);

    size_t getActiveSlotIndex(std::span<u8> data) const;

  public:
    SaveFile(const std::string &filename) : saveDataContainer{loadFile(filename)}, saveData{saveDataContainer}, patchedSaveData{saveDataContainer}, activeSlotIndex{getActiveSlotIndex(saveData)} {
        validateData(saveData, filename);
    }

    /**
     * @brief Write the patched save data to a file
     * @param filename The path to the save file
     */
    void write(const std::string &filename);

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
     * @brief Get the name of the character in save slot 0
     * @return The name of the character in save slot 0
     */
    std::string name(const std::span<u8> data) const;
    std::string name() const {
        return name(saveData);
    };

    /**
     * @brief Get the level of the character in the active slot
     * @return The level of the character
     */
    u32 level(const std::span<u8> data) const;
    u32 level() const {
        return level(saveData);
    };

    /**
     * @brief Check which save slot is active
     * @return The index of the active save slot
     */
    size_t activeSlot(const std::span<u8> data) const;
    size_t activeSlot() const {
        return activeSlot(saveData);
    };

    /**
     * @brief Get the checksum of the save header
     * @return The checksum of the save header
     */
    std::string checksum(const std::span<u8> data) const;
    std::string checksum() const {
        return checksum(saveData);
    };

    /**
     * @brief Get the amount of seconds played in the active slot
     * @return The amount of seconds played
     */
    std::string timePlayed(const std::span<u8> data) const;
    std::string timePlayed() const {
        return timePlayed(saveData);
    };
};

} // namespace savepatcher

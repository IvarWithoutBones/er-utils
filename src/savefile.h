#include "util.h"
#include <span>
#include <string>
#include <vector>

namespace savepatcher {

// A struct containing the offsets for save data sections
struct Section {
    size_t offset;
    size_t length;
    size_t size;

    constexpr Section(size_t offset, size_t size) : offset{offset}, size{size}, length{offset + size} {}

    /**
     * @brief Replace a section inside of the save file
     * @param data The data to modify
     * @param newSection The new data to replace the old section with
     */
    constexpr void replace(std::span<u8> data, const std::span<u8> newSection) const {
        if (offset < 0 || offset > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range while replacing: [{}, {}], size: {}", offset, length, data.size_bytes());
        if (newSection.size_bytes() != size)
            throw exception("New section size {} does not match old section size {}", newSection.size_bytes(), size);

        std::copy(newSection.begin(), newSection.end(), data.begin() + offset);
    }

    /**
     * @brief Get the range of bytes from the given data
     * @param data The data to return the range from
     * @return A span containing the range of bytes from the given offset
     */
    constexpr std::span<u8> bytesFrom(const std::span<u8> data) const {
        if (offset < 0 || offset > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range: [{}, {}], size: {}", offset, length, data.size_bytes());

        return data.subspan(offset, length - offset);
    }

    /**
     * @brief Get the range of bytes from the given data as a string
     * @param data The data to return the range from
     * @return A string containing the range of bytes from the given offset
     */
    std::string charsFrom(const std::span<u8> data) const {
        auto section{bytesFrom(data)};
        auto chars{static_cast<u8 *>(section.data())};
        return {chars, chars + section.size_bytes()};
    }

    /**
     * @brief Get the range of bytes from the given data as a hex string
     * @param data The data to return the range from
     * @return A string containing the range of bytes in an uppercase hex string
     */
    std::string hexFrom(const std::span<u8> data) const {
        return util::FormatHex(bytesFrom(data));
    }
};

/**
 * @brief Elden Ring save file parser and patcher
 * @param filename The path to the save file
 */
class SaveFile {
  private:
    constexpr static size_t SaveFileSize = 0x1BA03D0;                    //!< The size of the save file, used for validation
    constexpr static size_t SlotLength = 0x24C;                          //!< The length of a save slot
    constexpr static Section HeaderBNDSection{0x0, 0x3};                 //!< The section containing the characters BND, used for validation
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};      //!< The section containing the save header
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10}; //!< The section containing the checksum of the save header
    constexpr static Section SteamIdSection{0x19003B4, 0x8};             //!< The section containing the Steam ID
    constexpr static Section ActiveSection{0x1901D04, 0xA};              //!< The section containing a bool wether the save slot is active
    constexpr static Section NameSection{0x1901D0E, 0x22};               //!< The section containing the name of the character
    constexpr static Section LevelSection{0x1901D30, 0x1};               //!< The section containing the level of the character
    constexpr static Section SecondsPlayedSection{0x1901D34, 0x4};       //!< The section containing the amount of seconds played

    std::vector<u8> saveDataContainer; //!< The container the save data is stored in
    std::vector<u8> patchedSaveData;   //!< The save data to modify
    std::span<u8> saveData;            //!< A span of the original save data
    size_t activeSlotIndex;            //!< The index of the active slot in the save file

    /**
     * @brief Load a save file into memory
     * @param filename The path to the file
     */
    std::vector<u8> loadFile(const std::string &filename);

    /**
     * @brief Validate a file is an Elden Ring save file
     * @param data The data to validate
     * @param target The name of the file to validate, used for error messages
     */
    void validateData(std::span<u8> data, const std::string &target);

    /**
     * @brief Get the index of the active save slot, this can be a number between 0 and 10
     * @return The index of the first active save slot
     */
    size_t getActiveSlotIndex(std::span<u8> data) const;

  public:
    SaveFile(const std::string &filename) : saveDataContainer{loadFile(filename)}, saveData{saveDataContainer}, patchedSaveData{saveDataContainer} {
        validateData(saveData, filename);
        activeSlotIndex = getActiveSlotIndex(saveData);
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
    size_t level(const std::span<u8> data) const;
    size_t level() const {
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

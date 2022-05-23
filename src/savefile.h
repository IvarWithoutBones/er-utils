#include <span>
#include <string>
#include <vector>

namespace savepatcher {

// A struct containing the offsets for save data sections
struct Section {
    size_t offset;
    size_t length;

    constexpr Section(size_t offset, size_t length) : offset{offset}, length{offset + length} {}
};

class SaveFile {
  private:
    unsigned long steamId;

    // The offsets of the data sections
    constexpr static int SaveFileSize = 28967888;
    constexpr static Section HeaderBNDSection{0x0, 0x3};
    constexpr static Section SaveHeaderSection{0x19003B0, 0x60000};
    constexpr static Section SaveHeaderChecksumSection{0x19003A0, 0x10};
    constexpr static Section SteamIdSection{0x19003B4, 0x8};
    constexpr static Section ActiveSection{0x1901D04, 0x1};
    constexpr static Section NameSection{0x1901D0E, 0x22};

    // The raw data stored in the save file as raw bytes
    std::vector<uint8_t> originalSaveData;

    // A span to read the save file data from
    std::span<uint8_t> saveData;

    // The save data to write patched data to
    std::vector<uint8_t> patchedSaveData;

    // Load a save file from a file on disk
    std::vector<uint8_t> loadFile(const std::string &filename);

    // Validate a file is an Elden Ring save file
    void validateData(std::span<uint8_t> &data, const std::string &targetMsg);

    // Recalculate the save header checksum
    void recalculateChecksum();

    // Get the Steam ID from the save file
    unsigned long getSteamId() { return toLittleEndian(SteamIdSection); }

    // Get a range of raw bytes from the save data
    std::span<uint8_t> getByteRange(Section range, std::span<uint8_t> &data);
    std::span<uint8_t> getByteRange(Section range) { return getByteRange(range, saveData); }

    // Get a range of unformatted bytes from the save data as an ascii string
    std::string getCharRange(Section range, std::span<uint8_t> &data);
    std::string getCharRange(Section range) { return getCharRange(range, saveData); }

    // Convert a range of bytes to a little endian 16 bit integer
    unsigned long toLittleEndian(Section range, std::span<uint8_t> &data);
    unsigned long toLittleEndian(Section range) { return toLittleEndian(range, saveData); };

  public:
    SaveFile(const std::string &filename) {
        originalSaveData = loadFile(filename);
        patchedSaveData = originalSaveData;
        saveData = {originalSaveData.data(), originalSaveData.size()};

        steamId = getSteamId();
    }

    // Write the patched save data to a file
    void write(const std::string &filename);

    // Replace the steam ID hardcoded in the save file with the given steam ID
    void replaceSteamId(unsigned long steamId);

    // Get the name of the character in save slot 0
    std::string name() { return getCharRange(NameSection); }

    // Check wether save slot 0 is active or not
    bool active() { return getByteRange(ActiveSection)[0]; }
};

} // namespace savepatcher

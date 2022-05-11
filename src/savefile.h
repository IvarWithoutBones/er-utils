#include <string>
#include <vector>
#include <span>

namespace savepatcher {

// A struct containing the offsets for save data sections
struct Section {
    size_t offset;
    size_t length;

    constexpr Section(size_t offset, size_t length) : offset{offset}, length{offset + length} {}
};

class SaveFile {
  private:
    // The raw data stored in the save file as raw bytes
    std::vector<uint8_t> originalSavefile;

    // A span of the save file data
    std::span<uint8_t> dataSpan;

    // The offsets for the savefile sections
    constexpr static Section SteamIdSection{0x19003B4, 0x8};
    constexpr static Section ActiveSection{0x1901D04, 0x1};
    constexpr static Section SaveHeaderSection{0x1901D0E, 0x24C};
    constexpr static Section NameSection{SaveHeaderSection.offset, 0x22};

    // Load a save file from a file on disk
    std::vector<uint8_t> loadFile(const std::string& filename);

    // Get a range of raw bytes from the save data
    std::span<uint8_t> getByteRange(int beginOffset, size_t length);
    std::span<uint8_t> getByteRange(Section section) { return getByteRange(section.offset, section.length); }

    // Get a range of bytes from the save data as an ascii string
    std::string getCharRange(int beginOffset, size_t length);
    std::string getCharRange(Section section) { return getCharRange(section.offset, section.length); }

    // Convert a range of bytes to a little endian 16 bit integer
    unsigned long toLittleEndian(int beginOffset, size_t length);
    unsigned long toLittleEndian(Section section) { return toLittleEndian(section.offset, section.length); }

  public:
    SaveFile(const std::string& filename) {
        originalSavefile = loadFile(filename);
        dataSpan = {originalSavefile.data(), originalSavefile.size()};
    }

    // Get the name of the character in save slot 0
    std::string name();

    // Get the steam ID currently used by the save file
    unsigned long steamId();

    // Check wether save slot 0 is active or not
    bool active();

    // Replace the steam ID hardcoded in the save file with the given steam ID and return the patched data
    std::vector<uint8_t> replaceSteamId(unsigned long steamId);

    // Pretty(ish) print the saves binary data in hex
    void print(int beginOffset, size_t length, std::span<uint8_t> data);
    void print(std::span<uint8_t> range) { print(range.data()[0], range.size_bytes(), dataSpan); }
};

} // namespace savepatcher

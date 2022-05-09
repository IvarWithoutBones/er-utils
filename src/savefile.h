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
    // The containers for the save data. Note that only the span is being used directly
    std::vector<uint8_t> data;
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

    // Convert a value to a little endian 16 bit integer
    unsigned long long toLittleEndian(int beginOffset, size_t length);
    unsigned long long toLittleEndian(Section section) { return toLittleEndian(section.offset, section.length); }

  public:
    SaveFile(const std::string& filename) {
        data = loadFile(filename);
        dataSpan = {data.data(), data.size()};
    }

    // Get the name of the character in save slot 0
    std::string name();

    // Get the steam ID currently used by the save file
    unsigned long long steamId();

    // Check wether save slot 0 is active or not
    bool active();

    // Pretty(ish) print the saves binary data in hex
    void print(int beginOffset, size_t length);
    void print(size_t length) { print(0, length); };
    void print(const std::vector<uint8_t>& data) { print(0, data.size()); };
    void print() { print(0, data.size()); };
};

} // namespace savepatcher

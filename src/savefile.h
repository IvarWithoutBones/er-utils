#include <vector>
#include <string>

namespace savepatcher {

class SaveFile {
  private:
    // Load a save file from a file on disk
    std::vector<uint8_t> loadFile(const std::string& filename);
    std::vector<uint8_t> data;

    // Get a range of bytes from the save file
    std::vector<uint8_t> getByteRange(int beginOffset, int endOffset, std::vector<uint8_t>& section);
    std::vector<uint8_t> getByteRange(int beginOffset, int endOffset) { return getByteRange(beginOffset, endOffset, data); }
    std::vector<uint8_t> getByteRange(std::vector<uint8_t> data) { return getByteRange(0, data.size(), data); }
    std::vector<uint8_t> getByteRange(int offset) { return getByteRange(offset, offset + 1, data); }

    // Get a range of bytes from the save file as ascii
    std::string getCharRange(int beginOffset, int endOffset, std::vector<uint8_t>& section);
    std::string getCharRange(int beginOffset, int endOffset) { return getCharRange(beginOffset, endOffset, data); }

    // Convert a value to a little endian 16 bit integer
    unsigned long long toLittleEndian(int beginOffset, int endOffset, std::vector<uint8_t>& section);
    unsigned long long toLittleEndian(std::vector<uint8_t>& section) { return toLittleEndian(0, section.size(), section); }

    // The raw bytes pointing to the save header
    std::vector<uint8_t> saveHeader();

    // Copied from https://github.com/BenGrn/EldenRingSaveCopier/blob/v0.0.2-alpha/EldenRingSaveCopy/Saves/Model/SaveGame.cs#L11
    struct magic {
        int SAVE_HEADER_INDEX = 0x1901D0E;
        int SAVE_HEADER_LENGTH = 0x24C;
        int STEAM_ID_INDEX = 0x19003B4;
        int STEAM_ID_LENGTH = 0x8;
        int ACTIVE_INDEX = 0x1901D04;
        int NAME_OFFSET = 0x22;
        int LEVEL_OFFSET = 0x22;
        //int SLOT_START_INDEX = 0x310;
        //int SLOT_LENGTH = 0x280000;
        //int SAVE_HEADERS_SECTION_START_INDEX = 0x19003B0;
        //int SAVE_HEADERS_SECTION_LENGTH = 0x60000;
    } magic;

  public:
    SaveFile(const std::string& filename) {
        data = loadFile(filename);
    }

    // Get the name of the character in save slot 0
    std::string name();

    // Get the steam ID currently used by the save file
    unsigned long long steamId();

    // Check wether save slot 0 is active
    bool active();

    // Pretty(ish) print the save files binary data in hex
    void print(int beginOffset, int endOffset, std::vector<uint8_t>& data);
    void print(int beginOffset, int endOffset) { print(beginOffset, endOffset, data); }
    void print(int endOffset) { print(0, endOffset, data); };
    void print(std::vector<uint8_t> data) { print(0, data.size(), data); };
    void print() { print(0, data.size()); };
};

} // namespace savepatcher

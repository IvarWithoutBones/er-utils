#include <vector>
#include <string>

namespace savepatcher {

class SaveFile {
  private:
    // Load a save file from a file on disk
    std::vector<uint8_t> loadFile(const std::string& filename);
    std::vector<uint8_t> data;

    // Get a range of bytes from the save file
    std::vector<uint8_t> get(int beginOffset, int endOffset, std::vector<uint8_t>& data);
    std::vector<uint8_t> get(int beginOffset, int endOffset) { return get(beginOffset, endOffset, data); }
    std::vector<uint8_t> get(std::vector<uint8_t> data) { return get(0, data.size(), data); }
    std::vector<uint8_t> get(int offset) { return get(offset, offset + 1, data); }

    // The raw bytes from the save header
    std::vector<uint8_t> saveHeader();

    // copied from https://github.com/BenGrn/EldenRingSaveCopier/blob/v0.0.2-alpha/EldenRingSaveCopy/Saves/Model/SaveGame.cs#L11
    struct magic {
        int SAVE_HEADER_INDEX = 0x1901D0E;
        int SAVE_HEADER_LENGTH = 0x24C;
        int CHAR_NAME_LENGTH = 0x22;
        int CHAR_ACTIVE_INDEX = 0x1901D04;
        int CHAR_LEVEL_OFFSET = 0x22;
        int STEAM_ID_INDEX = 0x19003B4;
        int STEAM_ID_LENGTH = 0x8;
        //const static int SLOT_START_INDEX = 0x310;
        //const static int SLOT_LENGTH = 0x280000;
        //const static int SAVE_HEADERS_SECTION_START_INDEX = 0x19003B0;
        //const static int SAVE_HEADERS_SECTION_LENGTH = 0x60000;
    } magic;

  public:
    SaveFile(const std::string& filename) {
        data = loadFile(filename);
    }

    // Get the name of the character in save slot 0
    std::string name();

    // Get the steam ID currently used by the save file.
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

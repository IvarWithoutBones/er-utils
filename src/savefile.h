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
        const static int SAVE_HEADER_START_INDEX = 0x1901D0E;
        const static int SAVE_HEADER_LENGTH = 0x24C;
        const static int CHAR_NAME_LENGTH = 0x22;
        const static int CHAR_ACTIVE_START_INDEX = 0x1901D04;
        const static int CHAR_LEVEL_OFFSET = 0x22;

        //const static int SLOT_START_INDEX = 0x310;
        //const static int SLOT_LENGTH = 0x280000;
        //const static int SAVE_HEADERS_SECTION_START_INDEX = 0x19003B0;
        //const static int SAVE_HEADERS_SECTION_LENGTH = 0x60000;
        const static int STEAM_ID_LOCATION = 0x19003B4;
    } magic;

  public:
    SaveFile(const std::string& filename) {
        data = loadFile(filename);
    }

    // Get the name of the character in save slot 0
    std::string name();

    // Check wether save slot 0 is active
    bool isActive();

    // Pretty(ish) print the save files binary data in hex
    void print(int beginOffset, int endOffset, std::vector<uint8_t>& data);
    void print(int beginOffset, int endOffset) { print(beginOffset, endOffset, data); }
    void print() { print(0, data.size()); };
    void print(int startOffset) { print(startOffset, data.size()); };
};
} // namespace savepatcher

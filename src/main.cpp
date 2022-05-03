#include <fstream>
#include <fmt/core.h>

// copied from https://github.com/BenGrn/EldenRingSaveCopier/blob/v0.0.2-alpha/EldenRingSaveCopy/Saves/Model/SaveGame.cs#L11
const int SLOT_START_INDEX = 0x310;
const int SLOT_LENGTH = 0x280000;
const int SAVE_HEADERS_SECTION_START_INDEX = 0x19003B0;
const int SAVE_HEADERS_SECTION_LENGTH = 0x60000;
const int CHAR_ACTIVE_STATUS_START_INDEX = 0x1901D04;

const int SAVE_HEADER_START_INDEX = 0x1901D0E;
const int SAVE_HEADER_LENGTH = 0x24C;
const int CHAR_NAME_LENGTH = 0x22;
const int STEAM_ID_LOCATION = 0x19003B4;

class SaveFile {
  public:
    SaveFile(const std::string& filename) {
        data = load(filename);
    }

    // Get the name of the character in slot 0
    std::string getName() {
        std::vector<uint8_t> saveHeader = get(SAVE_HEADER_START_INDEX, SAVE_HEADER_START_INDEX + SAVE_HEADER_LENGTH);
        std::string name;

        for (int i = 0; i < CHAR_NAME_LENGTH; i++) {
            if (saveHeader[i] == 0)
                continue;

            name += static_cast<char>(saveHeader[i]);
        }

        return name;
    };

    bool isActive() {
        std::vector<uint8_t> charActiveStatus = get(CHAR_ACTIVE_STATUS_START_INDEX, CHAR_ACTIVE_STATUS_START_INDEX + 1);
        return charActiveStatus[0] == 1;
    };

    // Pretty(ish) print the save files binary data
    void print(int beginOffset, int endOffset) {
        for (int i = beginOffset; i < endOffset; i++) {
            auto byte = data[i];
            auto output = fmt::format("{:02x} ", byte & 0xff);

            if (i % 64 == 0 && i != 0)
                output += "\n";

            fmt::print(output);
        }

        // null termination
        fmt::print("\n");
    }

    void print() { print(0, data.size()); };
    void print(int startOffset) { print(startOffset, data.size()); };

    std::vector<uint8_t> get(int beginOffset, int endOffset) {
        if (beginOffset < 0 || endOffset > data.size() || beginOffset > endOffset) {
            throw std::runtime_error("Invalid offset range");
        }

        return std::vector<uint8_t>(data.begin() + beginOffset, data.begin() + endOffset);
    }
  private:
    std::vector<uint8_t> data;


    std::vector<uint8_t> load(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file");
        };

        std::string contents((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());


        // convert content to vector of binary data
        return std::vector<uint8_t> (contents.begin(), contents.end());
    };
};

int main(int argc, char** argv) {
    auto file = SaveFile("../steam.sl2");

    fmt::print("name: {}\n", file.getName());
    fmt::print("active: {}\n", file.isActive());
};

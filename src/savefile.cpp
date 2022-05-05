#include <fstream>
#include <bitset>
#include <fmt/format.h>
#include "savefile.h"

namespace savepatcher {

std::vector<uint8_t> SaveFile::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    };

    // Read the file into a byte array
    for (uint8_t byte; file.read(std::bit_cast<char*>(&byte), 1);) {
        data.push_back(byte);
    }

    file.close();

    // Validate the file is an Elden Ring save by checking if it starts with ascii "BND"
    for (int i = 0; i < 3; i++) {
        std::bitset<8> byte(data[i]);
        char ascii = char(byte.to_ulong());

        switch (ascii) {
            case 'B':
                break;
            case 'N':
                break;
            case 'D':
                break;
            default:
                auto errorMsg = fmt::format("File '{}' is not a valid Elden Ring save file.", filename);
                throw std::runtime_error(errorMsg);
        }
    };

    return data;
}

std::vector<uint8_t> SaveFile::get(int beginOffset, int endOffset, std::vector<uint8_t>& data) {
    if (beginOffset < 0 || endOffset > data.size() || beginOffset > endOffset) {
        throw std::runtime_error("Invalid offset range");
    }

    return std::vector<uint8_t>(data.begin() + beginOffset, data.begin() + endOffset);
}

std::vector<uint8_t> SaveFile::saveHeader() {
    return get(magic.SAVE_HEADER_START_INDEX, magic.SAVE_HEADER_START_INDEX + magic.SAVE_HEADER_LENGTH + 1);
};

std::string SaveFile::name() {
    std::string name;

    for (int i = 0; i < magic.CHAR_NAME_LENGTH; i++) {
        if (saveHeader()[i] == 0)
            continue;

        std::bitset<8> byte(saveHeader()[i]);
        char ascii = char(byte.to_ulong());

        name += ascii;
    }

    return name;
}

bool SaveFile::isActive() {
    return get(magic.CHAR_ACTIVE_START_INDEX)[0];
}

void SaveFile::print(int beginOffset, int endOffset, std::vector<uint8_t>& data) {
    std::vector<uint8_t> dataToPrint = get(beginOffset, endOffset, data);
    int index = 0;

    for (auto& i : dataToPrint) {
        auto byte = std::bitset<8>(i);
        auto output = fmt::format("{:02x} ", byte.to_ulong());

        if (index % 56 == 0 && index != 0)
            output += "\n";

        fmt::print(output);
        index++;
    }

    fmt::print("\n");
}

}; // namespace savepatcher

#include <fstream>
#include <bitset>
#include <fmt/format.h>
#include "savefile.h"

namespace savepatcher {

std::string SaveFile::name() {
    auto section = saveHeader();
    return getCharRange(0, magic.NAME_OFFSET, section);
}

bool SaveFile::active() {
    return getByteRange(magic.ACTIVE_INDEX)[0];
}

unsigned long long SaveFile::steamId() {
    auto section = getByteRange(magic.STEAM_ID_INDEX, magic.STEAM_ID_INDEX + magic.STEAM_ID_LENGTH);
    return toLittleEndian(section);
}

std::vector<uint8_t> SaveFile::saveHeader() {
    return getByteRange(magic.SAVE_HEADER_INDEX, magic.SAVE_HEADER_INDEX + magic.SAVE_HEADER_LENGTH + 1);
}

std::vector<uint8_t> SaveFile::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        auto errorMsg = fmt::format("Could not open file '{}'", filename);
        throw std::runtime_error(errorMsg);
    };

    // Read the file into a byte array
    for (uint8_t byte; file.read(std::bit_cast<char*>(&byte), 1);) {
        data.push_back(byte);
    }

    file.close();

    // Validate the file is an Elden Ring save by checking if it starts with ascii "BND"
    if (getCharRange(0, 3) != "BND") {
        auto errorMsg = fmt::format("File '{}' is not a valid Elden Ring save file.", filename);
        throw std::runtime_error(errorMsg);
    }

    return data;
}

std::vector<uint8_t> SaveFile::getByteRange(int beginOffset, int endOffset, std::vector<uint8_t>& section) {
    if (beginOffset < 0 || endOffset > section.size() || beginOffset > endOffset) {
        auto errorMsg = fmt::format("Invalid offset range: [{}, {}]", beginOffset, endOffset);
        throw std::runtime_error(errorMsg);
    }

    return std::vector<uint8_t>(section.begin() + beginOffset, section.begin() + endOffset);
}

std::string SaveFile::getCharRange(int beginOffset, int endOffset, std::vector<uint8_t>& section) {
    std::string chars;

    for (int i = beginOffset; i < endOffset; i++) {
        if (section[i] == 0)
            continue;

        std::bitset<8> byte(section[i]);
        char ascii = char(byte.to_ulong());
        chars += ascii;
    }

    return chars;
}

unsigned long long SaveFile::toLittleEndian(int beginOffset, int endOffset, std::vector<uint8_t>& section) {
    std::string output;

    // Convert the bytes to little endian by reversing their order
    std::vector<uint8_t> reversed = section;
    std::reverse(reversed.begin(), reversed.end());

    for (int i = beginOffset; i < endOffset; i++) {
        std::bitset<16> byte(reversed[i]);
        output += fmt::format("{:02x}", byte.to_ullong());
    };

    // Convert output to a 16 bit unsigned long long
    return std::stoul(output, nullptr, 16);
};

void SaveFile::print(int beginOffset, int endOffset, std::vector<uint8_t>& section) {
    std::vector<uint8_t> dataToPrint = getByteRange(beginOffset, endOffset, section);
    int index = 0;

    for (auto& i : dataToPrint) {
        auto byte = std::bitset<8>(i);
        auto output = fmt::format("{:02x} ", byte.to_ulong());

        // Arbitrary number of bytes per line
        if (index % 56 == 0 && index != 0)
            output += "\n";

        fmt::print(output);
        index++;
    }

    fmt::print("\n");
}

}; // namespace savepatcher

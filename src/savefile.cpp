#include <fstream>
#include <bitset>
#include <fmt/format.h>
#include "util.h"
#include "savefile.h"

namespace savepatcher {

std::string SaveFile::name() {
    return getCharRange(NameSection);
}

bool SaveFile::active() {
    return getByteRange(ActiveSection)[0];
}

unsigned long long SaveFile::steamId() {
    return toLittleEndian(SteamIdSection);
}

std::vector<uint8_t> SaveFile::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
        exception("Could not open file '{}'", filename);

    data = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();

    // Validate the file is an Elden Ring save by checking if it starts with ascii "BND"
    dataSpan = {data.data(), 3};
    if (getCharRange(0, 3) != "BND")
        exception("File '{}' is not a valid Elden Ring save file.", filename);

    return data;
}

std::span<uint8_t> SaveFile::getByteRange(int beginOffset, size_t length) {
    if (beginOffset < 0 || length > dataSpan.size() || beginOffset > length)
        exception("Invalid offset range: [{}, {}], size: {}", beginOffset, length, dataSpan.size_bytes());

    return dataSpan.subspan(beginOffset, length - beginOffset);
}

std::string SaveFile::getCharRange(int beginOffset, size_t size) {
    auto section{getByteRange(beginOffset, size)};
    std::string chars;

    for (auto& bit : section) {
        auto byte{std::bitset<8>(bit)};
        auto ascii{char(byte.to_ulong())};
        chars += ascii;
    }

    return chars;
}

unsigned long long SaveFile::toLittleEndian(int beginOffset, size_t length) {
    auto section{getByteRange(beginOffset, length)};
    std::vector<uint8_t> reversed;
    std::string output;

    // Copy span to an editable vector
    std::copy(section.begin(), section.end(), std::back_inserter(reversed));

    // Revert the order of the bytes as to convert it to little endian
    std::reverse(reversed.begin(), reversed.end());

    for (auto& bit : reversed) {
        std::bitset<16> byte(bit);
        output += fmt::format("{:02x}", byte.to_ullong());
    }

    // Convert output to a 16 bit unsigned long long
    return std::stoul(output, nullptr, 16);
}

void SaveFile::print(int beginOffset, size_t length) {
    auto dataToPrint{getByteRange(beginOffset, length)};
    std::string output;
    int index{};

    for (auto& i : dataToPrint) {
        auto byte{std::bitset<8>(i)};
        output += fmt::format("{:02x} ", byte.to_ulong());

        // Arbitrary number of bytes per line
        if (index % 56 == 0 && index != 0)
            output += "\n";

        index++;
    }

    fmt::print(fmt::runtime(output + "\n"));
}

}; // namespace savepatcher

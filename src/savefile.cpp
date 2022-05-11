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

unsigned long SaveFile::steamId() {
    return toLittleEndian(SteamIdSection);
}

std::vector<uint8_t> SaveFile::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
        exception("Could not open file '{}'", filename);

    originalSavefile = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    dataSpan = {originalSavefile.data(), 3};
    file.close();

    // Validate the file is an Elden Ring save by checking if it starts with ascii "BND"
    if (getCharRange(0, 3) != "BND")
        exception("File '{}' is not a valid Elden Ring save file.", filename);

    return originalSavefile;
}

std::span<uint8_t> SaveFile::getByteRange(int beginOffset, size_t length) {
    if (beginOffset < 0 || beginOffset > dataSpan.size_bytes() || length > (dataSpan.size_bytes() - beginOffset))
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

unsigned long SaveFile::toLittleEndian(int beginOffset, size_t length) {
    auto section{getByteRange(beginOffset, length)};
    std::vector<uint8_t> data;
    std::string output;

    // Copy the span to a mutable vector
    std::copy(section.begin(), section.end(), std::back_inserter(data));

    // Convert to little endian by reversing the order of the bytes
    std::reverse(data.begin(), data.end());

    for (auto& bit : data) {
        std::bitset<16> byte(bit);
        output += fmt::format("{:02x}", byte.to_ullong());
    }

    // Convert output to a 16 bit unsigned integer
    return std::stoul(output, nullptr, 16);
}

std::vector<uint8_t> SaveFile::replaceSteamId(unsigned long steamId) {
    std::vector<uint8_t> output{originalSavefile};
    std::vector<uint8_t> newSteamIdBytes;
    int idx{};

    // Convert the given Steam ID to raw bytes
    for (int i{}; i < 8; i++)
        newSteamIdBytes.emplace_back(steamId >> (i * 8));

    // Replace the steam ID inside of the save file
    for (int offset{SteamIdSection.offset}; offset < SteamIdSection.length; offset++) {
        //fmt::print("0x{}: replaced {:02x} with {:02x}\n", offset, output[offset], newSteamIdBytes[idx]);
        if (idx < newSteamIdBytes.size())
            output[offset] = newSteamIdBytes[idx];
        idx++;
    }

    idx = 0;

    // Write the output to a file
    // TODO: make configurable
    std::ofstream file("output.sl2", std::ios::binary);
    if (!file.is_open())
        exception("could not open file 'output.sl2'");
    fmt::print("writing output to: 'output.sl2'\n");

    file.clear();
    file.write(reinterpret_cast<char*>(output.data()), output.size());
    file.close();

    return output;
};

void SaveFile::print(int beginOffset, size_t length, std::span<uint8_t> data) {
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

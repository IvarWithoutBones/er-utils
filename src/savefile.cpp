#include <bitset>
#include <fstream>
#include <span>

#include <fmt/format.h>

#include "savefile.h"
#include "util.h"

namespace savepatcher {

std::vector<uint8_t> SaveFile::loadFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    originalSaveData = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();

    saveData = {originalSaveData.data(), originalSaveData.size()};
    validateData(saveData, fmt::format("File '{}'", filename));
    return originalSaveData;
}

void SaveFile::validateData(std::span<uint8_t> &data, const std::string &targetMsg) {
    bool valid{true};

    if (getCharRange(HeaderBNDSection, data) != "BND")
        valid = false;
    if (data.size_bytes() != SaveFileSize)
        valid = false;
    if (!valid)
        throw exception("{} is not a valid Elden Ring save file.", targetMsg);
}

void SaveFile::write(const std::string &filename) {
    std::ofstream file(filename, std::ios::binary);
    std::span<uint8_t> patchedData{patchedSaveData.data(), patchedSaveData.size()};

    // Validate if the data is correctly patched
    validateData(patchedData, "Generated data");

    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    file.write(reinterpret_cast<const char *>(patchedData.data()), patchedData.size_bytes());
    file.close();

    fmt::print("Succesfully wrote output to file '{}'\n", filename);
}

void SaveFile::replaceSteamId(unsigned long inputSteamId) {
    std::vector<uint8_t> newSteamIdBytes;
    int idx{};

    if (steamId != inputSteamId)
        fmt::print("Patching Steam ID: {} -> {}\n", steamId, inputSteamId);
    else
        throw exception("Steam ID is already correct");

    // Convert the given ID to raw bytes
    for (int i{}; i < 8; i++)
        newSteamIdBytes.emplace_back(inputSteamId >> (i * 8));

    // Replace the ID inside of the save file
    for (int offset{SteamIdSection.offset}; offset < SteamIdSection.length; offset++) {
        if (idx < newSteamIdBytes.size())
            patchedSaveData[offset] = newSteamIdBytes[idx];
        idx++;
    }

    recalculateChecksum();
};

void SaveFile::recalculateChecksum() {
    std::vector<uint8_t> saveHeaderData;
    std::string saveHeaderString;
    std::string checksumInFile;
    int idx{};

    // Read the checksum currently used in the save file
    for (int i{SaveHeaderChecksumSection.offset}; i < SaveHeaderChecksumSection.length; i++)
        checksumInFile += fmt::format("{:02x}", originalSaveData[i]);

    // Data of which the save header checksum is calculated
    for (int i{SaveHeaderSection.offset}; i < SaveHeaderSection.length; i++)
        saveHeaderData.emplace_back(patchedSaveData[i]);

    // Convert the data to a string for checksum calculation
    // TODO: use iterators maybe? probably faster, this is a ~3mb vector
    for (auto &byte : saveHeaderData) {
        std::bitset<8> bit(byte);
        saveHeaderString += bit.to_ulong();
    };

    // Calculate the MD5 checksum of the save header
    auto newChecksum{md5sum{saveHeaderString}};
    auto newChecksumString{newChecksum.getString()};
    auto newChecksumBytes{newChecksum.getBytes()};

    // Replace the checksum inside of the save file
    if (newChecksumString == checksumInFile)
        throw exception("Save header checksum is already correct");

    fmt::print("Patching save header checksum: {} -> {}\n", newChecksumString, checksumInFile);
    for (int i{SaveHeaderChecksumSection.offset}; i < SaveHeaderChecksumSection.length; i++) {
        if (idx < newChecksumBytes.size())
            patchedSaveData[i] = newChecksumBytes[idx];
        idx++;
    };
}

unsigned long SaveFile::toLittleEndian(Section range, std::span<uint8_t> &data) {
    auto section{getByteRange(range, data)};
    std::vector<uint8_t> bytes;
    std::string output;

    // Copy the span to a mutable vector
    std::copy(section.begin(), section.end(), std::back_inserter(bytes));

    // Convert to little endian by reversing the order of the bytes
    std::reverse(bytes.begin(), bytes.end());

    for (auto &bit : bytes) {
        std::bitset<16> byte(bit);
        output += fmt::format("{:02x}", byte.to_ullong());
    }

    // Convert output to a 16 bit unsigned integer
    return std::stoul(output, nullptr, 16);
}

std::span<uint8_t> SaveFile::getByteRange(Section range, std::span<uint8_t> &data) {
    if (range.offset < 0 || range.offset > data.size_bytes() || range.length > data.size_bytes())
        throw exception("Invalid offset range: [{}, {}], size: {}", range.offset, range.length, data.size_bytes());

    return data.subspan(range.offset, range.length - range.offset);
}

std::string SaveFile::getCharRange(Section range, std::span<uint8_t> &data) {
    auto section{getByteRange(range, data)};
    std::string chars;

    for (auto &bit : section) {
        auto byte{std::bitset<8>(bit)};
        auto ascii{char(byte.to_ulong())};
        chars += ascii;
    }

    return chars;
}

}; // namespace savepatcher

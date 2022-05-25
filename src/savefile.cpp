#include <bitset>
#include <fstream>
#include <span>

#include <fmt/format.h>

#include "savefile.h"
#include "util.h"

namespace savepatcher {

std::vector<u8> SaveFile::loadFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    originalSaveData = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    saveData = {originalSaveData.data(), originalSaveData.size()};
    file.close();

    validateData(saveData, fmt::format("File '{}'", filename));
    return originalSaveData;
}

void SaveFile::validateData(std::span<u8> &data, const std::string &target) {
    bool valid{true};

    if (getCharRange(HeaderBNDSection, data) != "BND")
        valid = false;
    if (data.size_bytes() != SaveFileSize)
        valid = false;

    if (!valid)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

void SaveFile::write(const std::string &filename) {
    std::ofstream file(filename, std::ios::binary);
    std::span<u8> patchedData{patchedSaveData.data(), patchedSaveData.size()};

    // Validate if the data is correctly patched
    validateData(patchedData, "Generated data");

    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    file.write(reinterpret_cast<const char *>(patchedData.data()), patchedData.size_bytes());
    file.close();

    fmt::print("Succesfully wrote output to file '{}'\n", filename);
}

void SaveFile::replaceSteamId(unsigned long inputSteamId) {
    std::array<u8, SteamIdSection.size> newSteamIdBytes;
    int idx{};

    if (steamId != inputSteamId)
        fmt::print("Patching Steam ID: {} -> {}\n", steamId, inputSteamId);
    else
        throw exception("Steam ID is already correct");

    // Convert the given ID to raw bytes
    for (int i{}; i < SteamIdSection.size; i++)
        newSteamIdBytes[i] = inputSteamId >> (i * bitsInByte);

    // Replace the ID inside of the save file
    for (int offset{SteamIdSection.offset}; offset < SteamIdSection.length; offset++) {
        if (idx < newSteamIdBytes.size())
            patchedSaveData[offset] = newSteamIdBytes[idx];
        idx++;
    }

    recalculateChecksum();
};

void SaveFile::recalculateChecksum() {
    std::array<u8, SaveHeaderSection.size> saveHeaderData;
    std::string saveHeaderString;
    std::string checksumFromFile;
    std::string newChecksumString;
    int idx{};

    // Data of which the save header checksum is calculated
    for (int i{SaveHeaderSection.offset}; i < SaveHeaderSection.length; i++) {
        saveHeaderData[idx] = patchedSaveData[i];
        idx++;
    };
    idx = 0;

    // Convert the data to a string for checksum calculation
    // TODO: use iterators maybe? probably faster, this is a ~3mb array
    for (auto &byte : saveHeaderData) {
        std::bitset<bitsInByte> bit(byte);
        saveHeaderString += bit.to_ulong();
    };

    // Calculate the MD5 checksum of the save header
    auto newChecksum{GenerateMd5(saveHeaderString)};

    for (auto &i : newChecksum)
        newChecksumString += fmt::format("{:02x}", i);
    for (int i{SaveHeaderChecksumSection.offset}; i < SaveHeaderChecksumSection.length; i++)
        checksumFromFile += fmt::format("{:02x}", originalSaveData[i]);

    if (newChecksumString == checksumFromFile)
        throw exception("Save header checksum is already correct");
    fmt::print("Patching save header checksum: {} -> {}\n", newChecksumString, checksumFromFile);

    // Replace the checksum inside of the save file
    for (int i{SaveHeaderChecksumSection.offset}; i < SaveHeaderChecksumSection.length; i++) {
        if (idx < newChecksum.size())
            patchedSaveData[i] = newChecksum[idx];
        idx++;
    };
}

unsigned long SaveFile::toLittleEndian(Section range, std::span<u8> &data) {
    auto section{getByteRange(range, data)};
    std::vector<u8> bytes;
    std::string output;

    // Copy the span to a mutable vector
    std::copy(section.begin(), section.end(), std::back_inserter(bytes));

    // Convert to little endian by reversing the order of the bytes
    std::reverse(bytes.begin(), bytes.end());

    for (auto &bit : bytes) {
        std::bitset<bitsInShort> i(bit);
        output += fmt::format("{:02x}", i.to_ullong());
    }

    // Convert output to a 16 bit unsigned integer
    return std::stoul(output, nullptr, 16);
}

std::span<u8> SaveFile::getByteRange(Section range, std::span<u8> &data) {
    if (range.offset < 0 || range.offset > data.size_bytes() || range.length > data.size_bytes())
        throw exception("Invalid offset range: [{}, {}], size: {}", range.offset, range.length, data.size_bytes());

    return data.subspan(range.offset, range.length - range.offset);
}

std::string SaveFile::getCharRange(Section range, std::span<u8> &data) {
    auto section{getByteRange(range, data)};
    std::string chars;

    for (auto &bit : section) {
        auto byte{std::bitset<bitsInByte>(bit)};
        auto ascii{char(byte.to_ulong())};
        chars += ascii;
    }

    return chars;
}

}; // namespace savepatcher

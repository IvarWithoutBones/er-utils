#include "savefile.h"
#include "util.h"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fstream>
#include <span>

namespace savepatcher {

void SaveFile::validateData(std::span<u8> data, const std::string &target) {
    if (HeaderBNDSection.charsFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

std::vector<u8> SaveFile::loadFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    std::vector<u8> data;
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    data = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();
    return data;
}

void SaveFile::write(std::span<u8> data, const std::string &filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    file.write(reinterpret_cast<const char *>(data.data()), data.size_bytes());
    file.close();
}

void SaveFile::copyCharacter(std::span<u8> source, std::span<u8> target, size_t charIndex) {
    std::vector<u8> result{target.data(), target.data() + target.size_bytes()};

    // Copy slot (correct)
    Section slotSection{0x310, 0x280000};
    slotSection.replace(result, slotSection.bytesFrom(source));

    // copy save header (correct)
    Section header{0x1901D0E, 0x24C};
    header.replace(result, header.bytesFrom(source));

    // replace save header checksum (correct)
    auto saveHeaderChecksum{GenerateMd5(SaveHeaderSection.bytesFrom(result))};
    SaveHeaderChecksumSection.replace(result, saveHeaderChecksum);

    // TODO: Replace the slot checksum at 0x300-0x310
    // This is supposed to be the MD5 sum of the slotSection, but somehow it's not matching up with other tools

    // TODO: 0x216A44-0x216A47 doesn't match up with other tools, while nothing seems to modify it?
    // The values emitted by other tools do not seem to occur in either the source or target file.

    write(result, "save.bin");
}

std::string SaveFile::recalculateChecksum() {
    auto saveHeaderChecksum{GenerateMd5(SaveHeaderSection.bytesFrom(patchedSaveData))};
    auto saveHeaderChecksumString{FormatHex(saveHeaderChecksum)};
    //if (checksum() == saveHeaderChecksumString)
    //    throw exception("Save header checksum is already correct");

    SaveHeaderChecksumSection.replace(patchedSaveData, saveHeaderChecksum);

    // Used for logging
    return saveHeaderChecksumString;
}

void SaveFile::replaceSteamId(u64 inputSteamId) {
    std::array<u8, SteamIdSection.size> newSteamIdBytes;
    // if (steamId() == inputSteamId)
    //     throw exception("Steam ID is already correct");

    // Convert the given ID to raw bytes
    // TODO: do this in a better way, I dont even understand what's going on here
    for (size_t i{}; i < SteamIdSection.size; i++)
        newSteamIdBytes[i] = inputSteamId >> (i * 8);

    SteamIdSection.replace(patchedSaveData, newSteamIdBytes);
}

std::string SaveFile::checksum(const std::span<u8> data) const {
    return FormatHex(SaveHeaderChecksumSection.bytesFrom(data));
}

u64 SaveFile::steamId(const std::span<u8> data) const {
    return SteamIdSection.castInteger<u64>(data);
}

std::vector<Character> SaveFile::parseCharacters(const std::span<u8, SaveFileSize> data) const {
    std::vector<Character> characters;
    for (size_t i{}; i < 10; i++) {
        auto character{Character{std::span<u8, SaveFileSize>(data), i}};
        if (character.active)
            characters.push_back(character);
    };

    return characters;
}

std::string Character::timePlayedFrom(std::span<u8, SaveFileSize> data) const {
    return SecondsToTimeStamp(SecondsPlayedSection.castInteger<u32>(data));
}

std::string Character::nameFrom(std::span<u8, SaveFileSize> data) const {
    return NameSection.charsFrom(data);
};

u64 Character::levelFrom(std::span<u8, SaveFileSize> data) const {
    return LevelSection.castInteger<u8>(data);
}

bool Character::activeFrom(std::span<u8, SaveFileSize> data, size_t slotIndex) const {
    return static_cast<bool>(ActiveSection.bytesFrom(data)[slotIndex]);
}

}; // namespace savepatcher

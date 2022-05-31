#include "savefile.h"
#include "util.h"
#include <fmt/format.h>
#include <fstream>
#include <span>

namespace savepatcher {

void SaveFile::validateData(std::span<u8> data, const std::string &target) {
    if (HeaderBNDSection.charsFrom(data) != "BND" || data.size_bytes() != SaveFileSize)
        throw exception("{} is not a valid Elden Ring save file.", target);
}

std::vector<u8> SaveFile::loadFile(const std::string &filename) {
    std::vector<u8> data;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    data = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    file.close();
    return data;
}

void SaveFile::write(const std::string &filename) {
    validateData(patchedSaveData, "Generated data");
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open())
        throw exception("Could not open file '{}'", filename);

    file.write(reinterpret_cast<const char *>(patchedSaveData.data()), patchedSaveData.size());
    file.close();
}

std::string SaveFile::recalculateChecksum() {
    auto saveHeaderChecksum{GenerateMd5(SaveHeaderSection.bytesFrom(patchedSaveData))};
    auto saveHeaderChecksumString{FormatHex(saveHeaderChecksum)};
    if (checksum() == saveHeaderChecksumString)
        throw exception("Save header checksum is already correct");

    SaveHeaderChecksumSection.replace(patchedSaveData, saveHeaderChecksum);

    // Used for logging
    return saveHeaderChecksumString;
}

void SaveFile::replaceSteamId(u64 inputSteamId) {
    std::array<u8, SteamIdSection.size> newSteamIdBytes;
    if (steamId() == inputSteamId)
        throw exception("Steam ID is already correct");

    // Convert the given ID to raw bytes
    // TODO: do this in a better way, I dont even understand what's going on here
    for (size_t i{}; i < SteamIdSection.size; i++)
        newSteamIdBytes[i] = inputSteamId >> (i * 8);

    SteamIdSection.replace(patchedSaveData, newSteamIdBytes);
}

u64 SaveFile::steamId(const std::span<u8> data) {
    auto section{SteamIdSection.bytesFrom(data)};
    if (section.size_bytes() % sizeof(u64))
        throw exception("Invalid Steam ID section size: {}", section.size_bytes());

    return *reinterpret_cast<u64 *>(section.data());
}

std::string SaveFile::checksum() const {
    return {SaveHeaderChecksumSection.hexFrom(saveData)};
}

bool SaveFile::active() const {
    return ActiveSection.bytesFrom(saveData)[0] == 1;
}

std::string SaveFile::name() const {
    return NameSection.charsFrom(saveData);
}

}; // namespace savepatcher

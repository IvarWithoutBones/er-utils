#include "savefile.h"
#include <fmt/format.h>

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto sourceFile{savepatcher::SaveFile("../backup/ER0000.backup1")};
    auto outputFile = "./output.sl2";
    auto steamId = 76561198257350685;

    for (auto &character : sourceFile.characters())
        fmt::print("{}: level {}, played for {}\n", character.name, character.level, character.timePlayed);

    sourceFile.replaceSteamId(steamId);
    fmt::print("Patched Steam ID: {} -> {}\n", sourceFile.steamId(), steamId);

    auto newChecksum{sourceFile.recalculateChecksum()};
    fmt::print("Patched save header checksum: {} -> {}\n", sourceFile.checksum(), newChecksum);

    sourceFile.write(outputFile);
    fmt::print("Succesfully wrote output to file '{}'\n", outputFile);
};

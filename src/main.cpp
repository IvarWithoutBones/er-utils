#include "savefile.h"
#include <fmt/format.h>

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto sourceFile{savepatcher::SaveFile("../steam.sl2")};
    auto outputFile = "./output.sl2";
    auto steamId = 76561198257350685;

    fmt::print("Active save slot: {}\n", sourceFile.activeSlot());
    fmt::print("Name: {}\n", sourceFile.name());
    fmt::print("Level: {}\n", sourceFile.level());

    sourceFile.replaceSteamId(steamId);
    fmt::print("Patched Steam ID: {} -> {}\n", sourceFile.steamId(), steamId);

    auto newChecksum{sourceFile.recalculateChecksum()};
    fmt::print("Patched save header checksum: {} -> {}\n", sourceFile.checksum(), newChecksum);

    sourceFile.write(outputFile);
    fmt::print("Succesfully wrote output to file '{}'\n", outputFile);
};

#include "savefile.h"
#include <fmt/format.h>

const void printActiveCharacters(std::vector<savepatcher::Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("  slot {}: {}, level {}, played for {}\n", character.getSlotIndex(), character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto outputFile = "./output.sl2";
    auto savefile{savepatcher::SaveFile("../saves/ashley.sl2", "../saves/backup/ER0000.backup1")};

    fmt::print("Source file with Steam ID: {}\n", savefile.steamId());
    printActiveCharacters(savefile.sourceCharacters);
    fmt::print("Target file with Steam ID: {}\n", savefile.steamId());
    printActiveCharacters(savefile.targetCharacters);

    savefile.appendSlot(1);
    savefile.write(outputFile);

    // TODO: specifying the second file shouldn't be required
    auto newfile{savepatcher::SaveFile(outputFile, "../er0000.sl2")};
    fmt::print("Generated file with Steam ID: {}\n", newfile.steamId());
    printActiveCharacters(newfile.sourceCharacters);

    fmt::print("Succesfully wrote output to file '{}'\n", outputFile);
};

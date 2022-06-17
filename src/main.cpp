#include "savefile.h"
#include <fmt/format.h>

using namespace savepatcher;

const void printActiveCharacters(std::vector<Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("  slot {}: {}, level {}, played for {}\n", character.getSlotIndex(), character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto sourceSave{SaveFile("../saves/ashley.sl2")};
    auto targetSave{SaveFile("../saves/backup/ER0000.backup1")};
    constexpr std::string_view outputFile = "./output.sl2";

    fmt::print("Savefile to copy from with Steam ID: {}\n", targetSave.steamId());
    printActiveCharacters(sourceSave.slots);

    fmt::print("Savefile to copy to with Steam ID: {}\n", sourceSave.steamId());
    printActiveCharacters(targetSave.slots);

    targetSave.copySlot(targetSave, 0, 2);
    targetSave.copySlot(sourceSave, 0, 0);
    targetSave.appendSlot(sourceSave, 0);

    fmt::print("Generated file:\n");
    printActiveCharacters(targetSave.slots);

    sourceSave.write(outputFile);
    fmt::print("Succesfully wrote output to file '{}'\n", outputFile);
};

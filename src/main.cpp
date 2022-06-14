#include "savefile.h"
#include <fmt/format.h>

const void printActiveCharacters(savepatcher::SaveFile &savefile) {
    for (auto character : savefile.characters)
        if (character.active)
            fmt::print("  {}: level {}, played for {}\n", character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto savefile{savepatcher::SaveFile("../saves/ashley.sl2", "../backup/ER0000.backup1")};
    auto outputFile = "./output.sl2";

    fmt::print("Source file with Steam ID: {}\n", savefile.steamId());
    printActiveCharacters(savefile);
    savefile.copyCharacter(0, 0);
    savefile.write(outputFile);

    // TODO: specifying the second file shouldn't be required
    auto newfile{savepatcher::SaveFile(outputFile, "../backup/ER0000.backup1")};
    fmt::print("Generated file with Steam ID: {}\n", newfile.steamId());
    printActiveCharacters(newfile);

    fmt::print("Succesfully wrote output to file '{}'\n", outputFile);
};

#include "arguments.h"
#include "savefile.h"
#include <fmt/format.h>

using namespace savepatcher;
using namespace savepatcher::arguments;

static void printActiveCharacters(std::vector<Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("  slot {}: {}, level {}, played for {}\n", character.getSlotIndex(), character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

static void printUsage(const ArgumentParser &parser) {
    const auto [programName, briefUsage, fullUsage] = parser.getUsage();
    fmt::print("Usage: {} {}\n{}\n", programName, briefUsage, fullUsage);
}

int main(int argc, char **argv) {
    ArgumentParser arguments(argc, argv);
    arguments.add<std::filesystem::path>({
        {"--source", "<save file>", "The path to the save file to copy from"},
        {"--target", "<save file>", "The path to the save file to copy to"},
        {"--output", "(optional) The path to the output file to write to"},
    });

    arguments.add<int>({
        {"--append", "<slot number>", "Append character with the given index to a save file"},
    });

    arguments.add<bool>({{"--help", "(optional) Show this help message"}});
    if (arguments.find<bool>("--help").value) {
        printUsage(arguments);
        exit(0);
    }

    const auto sourcePath{arguments.find<std::filesystem::path>("--source")};
    const auto targetPath{arguments.find<std::filesystem::path>("--target")};
    if (!sourcePath.set || !targetPath.set) { // TODO: add support for required arguments
        printUsage(arguments);
        exit(1);
    }

    auto sourceSave{SaveFile(sourcePath.value.generic_string())};
    auto targetSave{SaveFile(targetPath.value.generic_string())};

    fmt::print("Savefile to copy from with Steam ID: {}\n", targetSave.steamId());
    printActiveCharacters(sourceSave.slots);

    fmt::print("Savefile to copy to with Steam ID: {}\n", sourceSave.steamId());
    printActiveCharacters(targetSave.slots);

    auto appendArgument{arguments.find<int>("--append")};
    if (appendArgument.set)
        targetSave.appendSlot(sourceSave, appendArgument.value);

    fmt::print("Generated file:\n");
    printActiveCharacters(targetSave.slots);

    const auto outputPath{arguments.find<std::string_view>("--output")};
    if (outputPath.set) {
        sourceSave.write(outputPath.value);
        fmt::print("Succesfully wrote output to file '{}'\n", outputPath.value);
    }
}

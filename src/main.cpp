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

int main(int argc, char **argv) {
    ArgumentParser params(argc, argv);

    params.add<bool>({{"--help", "(optional) Show this help message"}});
    params.add<std::string_view>({ // TODO: should be std::filesystem::path instead
        {"--output", "(optional) The path to the output file to write to"},
        {"--source", "<save file>", "The path to the save file to copy from"},
        {"--target", "<save file>", "The path to the save file to copy to"},
    });

    params.add<size_t>({
        {"--append", "<slot number>", "Append character with the given index to a save file"},
    });

    if (params.get<bool>("--help")) {
        params.printUsage();
        exit(0);
    }

    const auto sourcePath{params.get<std::string_view>("--source")};
    const auto targetPath{params.get<std::string_view>("--target")};
    if (sourcePath.empty() || targetPath.empty()) { // TODO: add support for required arguments
        params.printUsage();
        exit(1);
    }

    auto sourceSave{SaveFile(sourcePath)};
    auto targetSave{SaveFile(targetPath)};

    fmt::print("Savefile to copy from with Steam ID: {}\n", targetSave.steamId());
    printActiveCharacters(sourceSave.slots);

    fmt::print("Savefile to copy to with Steam ID: {}\n", sourceSave.steamId());
    printActiveCharacters(targetSave.slots);

    // targetSave.copySlot(targetSave, 5, 1);
    // targetSave.setSlotActivity(1, false);
    // targetSave.setSlotActivity(5, true);
    // const auto slotIndex{params.get<size_t>("--append")};
    // targetSave.appendSlot(sourceSave, 0);
    targetSave.renameSlot(0, "test");

    fmt::print("Generated file:\n");
    printActiveCharacters(targetSave.slots);

    const auto outputPath{params.get<std::string_view>("--output")};
    if (!outputPath.empty()) {
        sourceSave.write(outputPath);
        fmt::print("Succesfully wrote output to file '{}'\n", outputPath);
    }
}

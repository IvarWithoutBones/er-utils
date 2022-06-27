#include "arguments.h"
#include "savefile.h"
#include <fmt/format.h>

using namespace savepatcher;
using namespace savepatcher::CommandLineArguments;

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
    std::string defaultPath;
    const std::filesystem::path appDataPath{fmt::format("{}/.steam/steam/steamapps/compatdata/1245620/pfx/drive_c/users/steamuser/AppData/Roaming/EldenRing", getenv("HOME"))};
    auto targetPath{arguments.add<std::string_view>({"--to", "<savefile>", "The path to the savefile to copy to"})};
    const auto sourcePath{arguments.add<std::string_view>({"--from", "<savefile>", "The path to the savefile to copy from"})};
    const auto read{arguments.add<std::string_view>({"--read", "<savefile>", "Print the slots of a save file"})};
    const auto output{arguments.add<std::string_view>({"--output", "<path>", "The path to write the generated file to"})};
    const auto append{arguments.add<int>({"--append", "<slot number>", "Append a slot from one savefile to another. '--from' needs to be set"})};
    if (arguments.add<bool>({"--help", "Show this help message"}).set) {
        printUsage(arguments);
        exit(0);
    }

    if (std::filesystem::exists(appDataPath)) {
        for (auto &path : std::filesystem::directory_iterator(appDataPath))
            if (path.is_directory()) {
                const auto filePath{path.path() / "ER0000.sl2"};
                if (std::filesystem::exists(filePath)) {
                    defaultPath = filePath.generic_string();
                    break;
                }
            }
    }

    if (read.set) {
        const auto path{std::filesystem::path{read.value}};
        auto saveFile{SaveFile{path}};
        fmt::print("Savefile '{}':\n", path.generic_string());
        printActiveCharacters(saveFile.slots);
        exit(0);
    }

    if (!targetPath.set) {
        if (!defaultPath.empty()) {
            targetPath.value = defaultPath;
            fmt::print("Found savefile at '{}'\n", defaultPath);
        } else {
            fmt::print("error: --to is required\n\n");
            printUsage(arguments);
            exit(1);
        }
    }
    auto targetSave{SaveFile(std::filesystem::path{targetPath.value})};
    if (argc == 1) {
        printActiveCharacters(targetSave.slots);
        exit(0);
    }

    if (!sourcePath.set && argc > 1) {
        fmt::print("error: --from is required\n\n");
        printUsage(arguments);
        exit(1);
    }
    auto sourceSave{SaveFile(std::filesystem::path{sourcePath.value})};
    fmt::print("Savefile to copy from:\n");
    printActiveCharacters(sourceSave.slots);
    fmt::print("Savefile to copy to:\n");
    printActiveCharacters(targetSave.slots);
    if (append.set) {
        if (sourceSave.slots[append.value].active)
            targetSave.appendSlot(sourceSave, append.value);
        else
            throw std::runtime_error(fmt::format("Attempting to append inactive slot {}", append.value));
    }

    fmt::print("Generated file:\n");
    printActiveCharacters(targetSave.slots);
    if (output.set) {
        const auto path{std::filesystem::path{output.value}};
        targetSave.write(path);
        fmt::print("Succesfully wrote output to file '{}'\n", util::toAbsolutePath(path));
    }
}

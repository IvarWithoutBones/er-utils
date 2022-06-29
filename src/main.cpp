#include "arguments.h"
#include "savefile.h"
#include "util.h"
#include <fmt/format.h>

using namespace savepatcher;
using namespace savepatcher::CommandLineArguments;

static void printActiveCharacters(std::vector<Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("  slot {}: {}, level {}, played for {}\n", character.slotIndex, character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

static void printUsage(const ArgumentParser &parser) {
    const auto [programName, briefUsage, fullUsage] = parser.getUsage();
    fmt::print("Usage: {} {}\n{}\n", programName, briefUsage, fullUsage);
}

int main(int argc, char **argv) {
    ArgumentParser arguments(argc, argv);
    std::string defaultFilePath;
    const std::filesystem::path appDataPath{fmt::format("{}/.steam/steam/steamapps/compatdata/1245620/pfx/drive_c/users/steamuser/AppData/Roaming/EldenRing", util::getEnvironmentVariable("HOME"))};

    auto targetPath{arguments.add<std::string_view>({"--to", "<savefile>", "The path to the savefile to copy to"})};
    const auto sourcePath{arguments.add<std::string_view>({"--from", "<savefile>", "The path to the savefile to copy from"})};
    const auto read{arguments.add<std::string_view>({"--read", "<savefile>", "Print the slots of a save file"})};
    const auto output{arguments.add<std::string_view>({"--output", "<path>", "The path to write the generated file to"})};
    const auto append{arguments.add<int>({"--append", "<slot number>", "Append a slot from one savefile to another. '--from' needs to be set"})};
    // TODO: restore argument
    const auto write{arguments.add<bool>({"--write", "Write the generated file to the Elden Ring folder to make it available to the game. A backup is written to '~/.config/er-saveutils'"})};
    if (arguments.add<bool>({"--help", "Show this help message"}).set) {
        printUsage(arguments);
        exit(0);
    }

    if (std::filesystem::exists(appDataPath)) {
        for (auto &path : std::filesystem::directory_iterator(appDataPath))
            if (path.is_directory()) {
                const auto filePath{path.path() / "ER0000.sl2"};
                if (std::filesystem::exists(filePath)) {
                    defaultFilePath = filePath.generic_string();
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
        if (!defaultFilePath.empty()) {
            targetPath.value = defaultFilePath;
            fmt::print("Found savefile at '{}'\n", defaultFilePath);
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

    if (append.set)
        fmt::print("Savefile to copy to:\n");
    printActiveCharacters(targetSave.slots);

    if (sourcePath.set) {
        auto sourceSave{SaveFile(std::filesystem::path{sourcePath.value})};
        fmt::print("Savefile to copy from:\n");
        printActiveCharacters(sourceSave.slots);

        if (append.set) {
            if (sourceSave.slots[append.value].active) {
                targetSave.appendSlot(sourceSave, append.value);
            } else
                throw std::runtime_error(fmt::format("Attempting to append inactive slot {}", append.value));
        }
    } else if (append.set) {
        fmt::print("error: --from is required\n\n");
        printUsage(arguments);
        exit(1);
    }

    if (append.set) {
        fmt::print("Generated file:\n");
        printActiveCharacters(targetSave.slots);

        if (output.set) {
            const auto path{std::filesystem::path{output.value}};
            targetSave.write(path);
            fmt::print("Succesfully wrote output to file '{}'\n", util::toAbsolutePath(path));
        }

        if (write.set) {
            if (defaultFilePath.empty())
                throw std::runtime_error("No savefile directory found to write to");

            auto backupPath{util::makeBackupDirectory() / "ER0000.sl2"};
            std::filesystem::copy(defaultFilePath, backupPath);
            targetSave.write(defaultFilePath);
            std::filesystem::remove(defaultFilePath + ".bak");
            fmt::print("Wrote backup of original savefile to '{}'\n", util::toAbsolutePath(backupPath));
            fmt::print("Wrote generated file to '{}'\n", util::toAbsolutePath(defaultFilePath));
        }
    }
}

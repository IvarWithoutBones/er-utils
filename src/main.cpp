#include "arguments.h"
#include "savefile.h"
#include "util.h"
#include <fmt/format.h>

#ifndef VERSION
#define VERSION "0.0.1"
#endif

using namespace savepatcher;

static void printActiveCharacters(std::vector<Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("    slot {}: {}, level {}, played for {}\n", character.slotIndex, character.name, character.level, character.timePlayed);
}

static void printUsage(const CommandLineArguments::ArgumentParser &parser) {
    const auto [programName, briefUsage, fullUsage] = parser.getUsage();
    fmt::print("Usage: {} {}\n{}\n", programName, briefUsage, fullUsage);
}

int main(int argc, char **argv) {
    CommandLineArguments::ArgumentParser arguments(argc, argv);
    const std::filesystem::path appDataPath{fmt::format("{}/.steam/steam/steamapps/compatdata/1245620/pfx/drive_c/users/steamuser/AppData/Roaming/EldenRing", util::getEnvironmentVariable("HOME"))};
    std::string defaultFilePath{util::findFileInSubDirectory(appDataPath, "ER0000.sl2")};

    auto targetPath{arguments.add<std::string_view>({"--to", "<savefile>", "The path to the savefile to copy to. By default the folder from Steam is used"})};
    const auto sourcePath{arguments.add<std::string_view>({"--from", "<savefile>", "The path to the savefile to copy from"})};
    const auto read{arguments.add<std::string_view>({"--read", "<savefile>", "Print the slots of a savefile"})};
    const auto import{arguments.add<std::string_view>({"--import", "<savefile>", "Patch a savefiles Steam ID and copy it to the games directory"})};
    const auto output{arguments.add<std::string_view>({"--output", "<path>", "The path to write the generated file to"})};
    const auto append{arguments.add<int>({"--append", "<slot number>", "Append a slot from one savefile to another. '--from' needs to be set"})};
    auto steamId{arguments.add<u64>({"--steamid", "<Steam ID>", "Set a Steam ID to patch the savefile with, in case it cannot be detected automatically"})};
    // TODO: restore argument
    const auto write{arguments.add<bool>({"--write", "Write the generated file to Steams Elden Ring folder to make it available to the game. A backup of the existing savefile gets written to '~/.config/er-saveutils/backup'"})};
    auto rename{arguments.add<std::tuple<int, std::string_view>>({"--rename", "<slot number> <name>", "Rename a slot in the savefile"})};
    auto listItems{arguments.add<int>({"--list-items", "<slot number>", "List all items in the given slot"})};
    auto setItem{arguments.add<std::tuple<int, std::string_view, u32>>({"--set-item", "<slot number> <item name> <item count>", "Set the number of a given item in a slot. To see a list of all available items, use --list-items"})};
    auto help{arguments.add<bool>({"--help", "Print this help message"})};
    auto version{arguments.add<bool>({"--version", "Print the version of the program"})};
    arguments.checkForUnexpected();

    if (help.set) {
        printUsage(arguments);
        exit(0);
    }

    if (version.set) {
        fmt::print("er-saveutils v{}\n", VERSION);
        exit(0);
    }

    if (read.set) {
        const auto path{std::filesystem::path{read.value}};
        auto saveFile{SaveFile{path}};
        fmt::print("Savefile '{}':\n", path.generic_string());
        printActiveCharacters(saveFile.slots);
        exit(0);
    }

    if (!targetPath.set && !import.set) {
        if (!defaultFilePath.empty()) {
            targetPath.value = defaultFilePath;
            fmt::print("Found savefile at '{}'\n", defaultFilePath);
        } else {
            fmt::print("error: Could not find the savefile from Steam, --to is required\n\n");
            printUsage(arguments);
            exit(1);
        }
    }

    if (import.set)
        targetPath.value = import.value;

    auto targetSave{SaveFile(std::filesystem::path{targetPath.value})};

    if (rename.set) {
        const auto [slotIndex, name] = rename.value;
        fmt::print("Renaming slot {} to '{}'\n", slotIndex, name);
        if (!targetSave.slots[slotIndex].active)
            throw exception("Slot {} is not active while renaming", slotIndex);
        targetSave.renameSlot(slotIndex, name);
    }

    if (import.set)
        if (!steamId.set)
            steamId.value = util::getSteamId(defaultFilePath);

    if (import.set || steamId.set) {
        if (util::getDigits(steamId.value) != 17)
            throw exception("Invalid Steam ID: {}", steamId.value);
        fmt::print("Patching '{}' with Steam ID {}\n", targetPath.value, steamId.value);
        targetSave.replaceSteamId(steamId.value);
    }

    if (append.set)
        fmt::print("Savefile to copy to:\n");
    if (listItems.set || setItem.set)
        fmt::print("Slots:\n", listItems.value);

    printActiveCharacters(targetSave.slots);

    if (setItem.set) {
        Items items{};
        const auto [slotIndex, itemName, itemCount] = setItem.value;
        auto item{items[itemName]};
        fmt::print("Setting item {} to {} in slot {}\n", itemName, itemCount, slotIndex);
        if (!targetSave.slots[slotIndex].active)
            throw exception("Slot {} is not active while setting item", slotIndex);
        if (targetSave.getItem(slotIndex, item) == 0)
            throw exception("Could not set item {} to {} in slot {} as it is not present in the savefile", itemName, itemCount, slotIndex);
        targetSave.setItem(slotIndex, item, itemCount);
    }

    if (listItems.set) {
        Items items{};
        fmt::print("Items in slot {}:\n", listItems.value);
        if (!targetSave.slots[listItems.value].active)
            throw exception("Slot {} is not active while listing items", listItems.value);
        for (auto item : items)
            fmt::print("    {}: {}\n", item.first, targetSave.getItem(listItems.value, item.second));
    }

    if (sourcePath.set) {
        auto sourceSave{SaveFile(std::filesystem::path{sourcePath.value})};
        fmt::print("Savefile to copy from:\n");
        printActiveCharacters(sourceSave.slots);

        if (append.set) {
            if (sourceSave.slots[append.value].active)
                targetSave.appendSlot(sourceSave, append.value);
            else
                throw exception("Attempting to append inactive slot {}", append.value);
        }
    } else if (append.set) {
        fmt::print("error: Could not find a savefile, please set one manually with '--from'\n\n");
        printUsage(arguments);
        exit(1);
    }

    if (append.set) {
        fmt::print("Generated file:\n");
        printActiveCharacters(targetSave.slots);
    }

    if (output.set) {
        const auto path{std::filesystem::path{output.value}};
        targetSave.write(path);
        fmt::print("Succesfully wrote output to file '{}'\n", util::toAbsolutePath(path));
    }

    if (write.set || import.set) {
        if (defaultFilePath.empty())
            throw exception("No savefile directory found to write to");

        auto backupDir{util::backupAndRemoveSavefile(defaultFilePath)};
        fmt::print("Wrote a backup of the original savefile to '{}'\n", util::toAbsolutePath(backupDir));
        targetSave.write(defaultFilePath);
        fmt::print("Wrote generated file to '{}'\n", util::toAbsolutePath(defaultFilePath));
    }
}

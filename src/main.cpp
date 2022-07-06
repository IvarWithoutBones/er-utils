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

    auto targetPath{arguments.add<std::string_view>({"--to", "<savefile>", "The path to the savefile to edit, refered to as the target path. Note that this file will not get overwritten unless '--write' is set. By default the file from Steam is used"})};
    auto sourcePath{arguments.add<std::string_view>({"--from", "<savefile>", "The path to the savefile to copy from when appending"})};
    auto read{arguments.add<std::string_view>({"--read", "<savefile>", "Print the slots of a savefile"})};
    auto import{arguments.add<std::string_view>({"--import", "<savefile>", "Patch a savefiles Steam ID and copy it to Steams directory to make it available to the game"})};
    auto output{arguments.add<std::string_view>({"--output", "<path>", "The path to write the generated file to after editing"})};
    auto append{arguments.add<int>({"--append", "<slot number>", "Append a slot from the save file set with '--from' to the save file set with '--to'"})};
    auto slot{arguments.add<int>({"--slot", "<slot number>", "The slot to use when editing the save file"})};
    auto steamId{arguments.add<u64>({"--steamid", "<Steam ID>", "Set a Steam ID to patch the savefile with, in case it cannot be detected automatically"})};
    auto rename{arguments.add<std::string_view>({"--rename", "<name>", "Rename a slot in the savefile. '--slot' must be set"})};
    auto listItems{arguments.add<bool>({"--list-items", "List all items in the slot from '--slot'"})};
    auto setItem{arguments.add<std::pair<std::string_view, u32>>({"--set-item", "<item name> <item count>", "Set the number of a given item in the slot set with '--slot`. To see a list of all available items, use --list-items"})};
    auto help{arguments.add<bool>({"--help", "Print this help message"})};
    auto version{arguments.add<bool>({"--version", "Print the version of the program"})};
    // TODO: restore argument
    auto write{arguments.add<bool>({"--write", "Write the generated file to Steams Elden Ring folder to make it available to the game. A backup of the existing savefile gets written to '~/.config/er-saveutils/backup'"})};
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
        auto name{rename.value};
        if (!slot.set)
            throw exception("--rename requires --slot to be set to the slot to rename");

        fmt::print("Renaming slot {} to '{}'\n", slot.value, name);
        if (!targetSave.slots[slot.value].active)
            throw exception("Slot {} is not active while renaming", slot.value);
        targetSave.renameSlot(slot.value, name);
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

    printActiveCharacters(targetSave.slots);

    if (setItem.set) {
        Items items{};
        const auto [itemName, itemCount] = setItem.value;
        auto item{items[itemName]};
        if (!slot.set)
            throw exception("--set-item requires --slot to be set to the slot to edit");
        if (!targetSave.slots[slot.value].active)
            throw exception("Slot {} is not active while setting item {}", slot.value, itemName);
        fmt::print("Setting {} to {} in slot {}\n", itemName, itemCount, slot.value);
        targetSave.setItem(slot.value, item, itemCount);
    }

    if (listItems.set) {
        Items items{};
        fmt::print("Items in slot {}:\n", slot.value);
        if (!slot.set)
            throw exception("--list-items requires --slot to be set to the slot to list");
        if (!targetSave.slots[slot.value].active)
            throw exception("Slot {} is not active while listing items", slot.value);
        for (auto item : items)
            if (targetSave.getItem(slot.value, item.second) != 0)
                fmt::print("    {}: {}\n", item.first, targetSave.getItem(slot.value, item.second));
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

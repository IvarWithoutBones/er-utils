#include "arguments.h"
#include "savefile.h"
#include "util.h"
#include <fmt/color.h>
#include <fmt/format.h>

#ifndef VERSION
#define VERSION "0.0.1"
#endif

int main(int argc, char **argv) {
    CommandLineArguments::ArgumentParser arguments(argc, argv);
    const std::filesystem::path appDataPath{fmt::format("{}/.steam/steam/steamapps/compatdata/1245620/pfx/drive_c/users/steamuser/AppData/Roaming/EldenRing", util::GetEnvironmentVariable("HOME"))};
    std::string defaultFilePath{util::FindFileInSubDirectory(appDataPath, "ER0000.sl2")};

    auto targetPath{arguments.add<std::string_view>({"--to", "<savefile>", "The path to the savefile to edit, refered to as the target path. Note that this file will not get overwritten unless '--write' is set. By default the file from Steam is used"})};
    auto sourcePath{arguments.add<std::string_view>({"--from", "<savefile>", "The path to the savefile to copy from when appending"})};
    auto import{arguments.add<std::string_view>({"--import", "<savefile>", "Patch a savefiles Steam ID and copy it to Steams directory to make it available to the game"})};
    auto output{arguments.add<std::string_view>({"--output", "<path>", "The path to write the generated file to after editing"})};
    auto append{arguments.add<int>({"--append", "<slot number>", "Append a slot from the save file set with '--from' to the save file set with '--to'"})};
    auto slot{arguments.add<u8>({"--slot", "<slot number>", "The slot to use when editing the save file"})};
    auto steamId{arguments.add<u64>({"--steamid", "<Steam ID>", "Set a Steam ID to patch the savefile with, in case it cannot be detected automatically. This should be a number with 17 digits"})};
    auto rename{arguments.add<std::string_view>({"--rename", "<new_name>", "Rename the slot set with '--slot'"})};
    auto listItems{arguments.add<bool>({"--list-items", "List all items in the slot from '--slot'"})};
    auto debugListItems{arguments.add<bool>({"--debug-list-items", "List all the items that are not yet recognized"})};
    auto setItem{arguments.add<std::pair<std::string_view, u32>>({"--set-item", "<item name> <item count>", "Set the number of a given item in the slot set with '--slot`. To see a list of all available items, use --list-items"})};
    auto help{arguments.add<bool>({"--help", "Print this help message"})};
    auto version{arguments.add<bool>({"--version", "Print the version of the program"})};
    // TODO: restore argument
    auto write{arguments.add<bool>({"--write", "Write the generated file to Steams Elden Ring folder to make it available to the game. A backup of the existing savefile gets written to '~/.config/er-saveutils/backup'"})};
    arguments.checkForUnexpected();

    if (help.set)
        arguments.exitAndShowUsage();

    if (version.set) {
        fmt::print("er-saveutils v{}\n", VERSION);
        exit(0);
    }

    if (!targetPath.set && !import.set) {
        if (!defaultFilePath.empty()) {
            targetPath.value = defaultFilePath;
            fmt::print("Found savefile at '{}'\n", defaultFilePath);
        } else
            arguments.exitAndShowUsage(targetPath, "Could not find the save file in Steams directory, setting --to is required", 1);
    }

    if (import.set)
        targetPath.value = import.value;

    auto targetSave{SaveFile(std::filesystem::path{targetPath.value})};

    if (debugListItems.set) {
        if (!slot.set)
            arguments.exitAndShowUsage(debugListItems, "--debug-list-items requires --slot to be set", 1);
        targetSave.debugListItems(slot.value);
    }

    if (rename.set) {
        auto name{rename.value};
        if (!slot.set)
            arguments.exitAndShowUsage(rename, "--rename requires --slot to be set", 1);

        fmt::print("Renaming slot {} to '{}'\n", slot.value, name);
        if (!targetSave.slots[slot.value].active)
            arguments.exitAndShowUsage(rename, fmt::format("Slot {} is not active while trying to rename", slot.value), 1);
        targetSave.renameSlot(slot.value, name);
    }

    if (import.set)
        if (!steamId.set)
            steamId.value = util::GetSteamId(defaultFilePath);

    if (import.set || steamId.set) {
        if (util::getDigits(steamId.value) != 17)
            arguments.exitAndShowUsage(steamId, fmt::format("Invalid Steam ID: {}", steamId.value), 1);
        fmt::print("Patching '{}' with Steam ID {}\n", targetPath.value, steamId.value);
        targetSave.replaceSteamId(steamId.value);
    }

    if (append.set)
        fmt::print("Savefile to copy to:\n");

    targetSave.printActiveSlots();

    if (setItem.set) {
        Items::Items items{};
        const auto [itemName, itemCount] = setItem.value;
        auto item{items[itemName]};
        if (!slot.set)
            arguments.exitAndShowUsage(setItem, "--set-item requires --slot to be set", 1);
        if (!targetSave.slots[slot.value].active)
            arguments.exitAndShowUsage(setItem, fmt::format("Slot {} is not active while trying to set an items quantity", slot.value), 1);
        fmt::print("Setting {} to {} in slot {}\n", itemName, itemCount, slot.value);
        targetSave.setItem(slot.value, item, itemCount);
    }

    if (listItems.set) {
        Items::Items items{};
        fmt::print("Items in slot {}:\n", slot.value);
        if (!slot.set)
            arguments.exitAndShowUsage(listItems, "--list-items requires --slot to be set", 1);
        if (!targetSave.slots[slot.value].active)
            arguments.exitAndShowUsage(listItems, fmt::format("Slot {} is not active while trying to list items", slot.value), 1);
        for (auto item : items)
            if (targetSave.getItem(slot.value, item.second) != 0)
                fmt::print("    {}: {}\n", item.first, targetSave.getItem(slot.value, item.second));
    }

    if (sourcePath.set) {
        auto sourceSave{SaveFile(std::filesystem::path{sourcePath.value})};
        fmt::print("Savefile to copy from:\n");
        sourceSave.printActiveSlots();

        if (append.set) {
            if (sourceSave.slots[append.value].active)
                targetSave.appendSlot(sourceSave, append.value);
            else
                arguments.exitAndShowUsage(append, fmt::format("Slot {} is not active while trying to append", append.value), 1);
        }
    } else if (append.set)
        arguments.exitAndShowUsage(append, "--append requires --from to be set to the path to the savefile to copy from", 1);

    if (append.set) {
        fmt::print("Generated file:\n");
        targetSave.printActiveSlots();
    }

    if (output.set) {
        const auto path{std::filesystem::path{output.value}};
        targetSave.write(path);
        fmt::print("Succesfully wrote output to file '{}'\n", util::ToAbsolutePath(path).generic_string());
    }

    if (write.set || import.set) {
        if (defaultFilePath.empty() && !targetPath.set)
            arguments.exitAndShowUsage(write, "No location known to write to as Steams directory was not found, and '--to' was not set", 1);

        auto backupDir{util::BackupSavefile(defaultFilePath)};
        fmt::print("Wrote a backup of the original savefile to '{}'\n", util::ToAbsolutePath(backupDir).generic_string());
        targetSave.write(defaultFilePath);
        fmt::print("Wrote generated file to '{}'\n", util::ToAbsolutePath(defaultFilePath).generic_string());
    }
}

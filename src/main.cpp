#include "arguments.h"
#include "savefile.h"
#include "util.h"
#include <fmt/format.h>

#ifndef VERSION
#define VERSION "0.0.1"
#endif

int main(int argc, char **argv) {
    CommandLineArguments::ArgumentParser arguments(argc, argv);
    std::string defaultSavePath{util::FindFileInSubDirectory(fmt::format("{}/.steam/steam/steamapps/compatdata/1245620/pfx/drive_c/users/steamuser/AppData/Roaming/EldenRing", util::GetEnvironmentVariable("HOME")), "ER0000.sl2")};
    std::filesystem::path outputPath;
    bool shownSlots{false};

    auto save{arguments.add<std::string_view>({"--save", "<savefile>", "The savefile to edit, by default this is the savefile found in Steams AppData directory"})};
    auto steamId{arguments.add<u64>({"--steam-id", "<Steam ID>", "Replace the Steam ID embedded in the savefile. This should be a number with 17 digits"})}; // TODO: validation
    auto slot{arguments.add<int>({"--slot", "<slot number>", "The index of the slot to edit, by default the first. Use --show to list all available options"})};
    auto show{arguments.add<bool>({"--show", "View information about all active slots"})};
    auto rename{arguments.add<std::string_view>({"--rename", "<new name>", "Rename the character in the specified slot"})};
    auto copy{arguments.add<int>({"--copy", "<slot number>", "Copy the slot specified by '--slot' to a new slot"})};
    auto import{arguments.add<std::pair<std::string_view, int>>({"--import", "<savefile> <slot number>", "Import a slot from a different savefile into the slot specified with '--slot'"})};
    auto listAllItems{arguments.add<bool>({"--list-all-items", "List all the items that this program can edit"})};
    auto listItems{arguments.add<bool>({"--list-items", "List all items collected in the specified slot"})};
    auto setItem{arguments.add<std::pair<std::string_view, u32>>({"--set-item", "<item name> <amount>", "Change the amount of an item in the specified slot"})};
    auto debugListItems{arguments.add<bool>({"--debug-list-items", "List all the items that are not yet implemented, useful for debugging"})};
    auto output{arguments.add<std::string_view>({"--output", "<savefile>", "Write the edited savefile to a new file"})};
    auto dryRun{arguments.add<bool>({"--dry-run", "Do not write any changes to the savefile"})};
    auto version{arguments.add<bool>({"--version", "Print the version of the program"})};
    auto help{arguments.add<bool>({"--help", "Print this help message"})};
    arguments.check();

    // TODO: default values in the argument parser
    if (!slot.set)
        slot.value = 0;
    if (!save.set)
        save.value = defaultSavePath;

    if (help.set) {
        arguments.showUsage();
        return 0;
    } else if (version.set) {
        fmt::print("erutils v{}\n", VERSION);
        return 0;
    }

    SaveFile saveFile{save.value};
    fmt::print("using savefile '{}'\nSteam ID embedded in the savefile: {}\n", save.value, saveFile.steamId());

    if (arguments.size() == 0) {
        saveFile.printActiveSlots();
        fmt::print("\nuse --help to see all available options\n");
        exit(0);
    }

    if (steamId.set) {
        saveFile.replaceSteamId(steamId.value);
        fmt::print("Steam ID set to {}\n", steamId.value);
    }
    fmt::print("\n");

    if (import.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        SaveFile importFile{import.value.first};
        saveFile.copySlot(importFile, import.value.second, slot.value);
        fmt::print("imported slot {} from savefile '{}' into slot {}\n\n", import.value.second, import.value.first, slot.value);
    }

    if (rename.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        saveFile.renameSlot(slot.value, rename.value);
        fmt::print("renamed slot {} to '{}'\n\n", slot.value, rename.value);
    }

    if (copy.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        saveFile.copySlot(slot.value, copy.value);
        fmt::print("copied slot {} to slot {}\n\n", slot.value, copy.value);
    }

    if (show.set) {
        shownSlots = true;
        saveFile.printActiveSlots();
    }

    if (setItem.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        saveFile.setItem(slot.value, saveFile.items[setItem.value.first], setItem.value.second);
        fmt::print("set item '{}' to {}\n\n", setItem.value.first, setItem.value.second);
    }

    if (listItems.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        fmt::print("all items in slot {}:\n\n", slot.value);
        saveFile.printItems(slot.value);
        fmt::print("\n");
    }

    if (listAllItems.set) {
        fmt::print("all items available to edit:\n\n");
        saveFile.items.print();
        fmt::print("\n");
    }

    if (debugListItems.set) {
        if (!shownSlots) {
            saveFile.printSlot(slot.value);
            shownSlots = true;
        }
        fmt::print("all unrecognized items in slot {}:\n\n", slot.value);
        saveFile.debugListItems(slot.value);
        fmt::print("\n");
    }

    if (!shownSlots)
        saveFile.printActiveSlots();

    fmt::print("\n");
    if (!dryRun.set) {
        auto backupFile{util::BackupSavefile(save.value)};
        // TODO: detect if the savefile has changed since it was loaded
        fmt::print("wrote a backup of the original savefile to '{}'\n", util::ToAbsolutePath(backupFile).generic_string());
        if (output.set) {
            outputPath = output.value;
            if (std::filesystem::exists(outputPath))
                fmt::print("the output file '{}' already exists, overwriting it\n", outputPath.generic_string());
        } else
            outputPath = save.value;
        saveFile.write(outputPath);
        fmt::print("succesfully wrote changes to '{}'\n", outputPath.generic_string());
    }
}

#include <fmt/format.h>

#include "savefile.h"

// TODO: Command line arguments
int main(int argc, char **argv) {
    auto file{savepatcher::SaveFile("../pirated.sl2")};

    fmt::print("Name: {}\n", file.name());
    fmt::print("Slot 0 active: {}\n", file.active());

    file.replaceSteamId(76561198257350685);
    file.write("./output.sl2");
};

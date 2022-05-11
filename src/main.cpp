#include <fmt/format.h>
#include "savefile.h"

// TODO: Command line arguments
int main(int argc, char** argv) {
    auto file{savepatcher::SaveFile("../steam.sl2")};

    fmt::print("name: {}\n", file.name());
    fmt::print("slot 0 active: {}\n", file.active());
    fmt::print("steam id: {}\n", file.steamId());

    file.replaceSteamId(66561198257350681);
};
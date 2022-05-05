#include <fmt/format.h>
#include "savefile.h"

int main(int argc, char** argv) {
    auto file = savepatcher::SaveFile("../steam.sl2");

    file.print();

    fmt::print("name: {}\n", file.name());
    fmt::print("active: {}\n", file.isActive());
};

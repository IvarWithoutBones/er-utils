#include <fstream>
#include <fmt/core.h>

// https://www.reddit.com/r/PiratedGames/comments/t3s53o/elden_ring_cracked_game_save_to_steam/

int main(int argc, char** argv) {
    std::string saveFilePath = "../ER0000.sl2";
    
    std::ifstream savefileStream(saveFilePath, std::ios::binary);

    // read ifstream as binary
    std::string savefileContents((std::istreambuf_iterator<char>(savefileStream)),
                                  std::istreambuf_iterator<char>());

    // convert string to binary data
    std::vector<unsigned int> savefile(savefileContents.begin(), savefileContents.end());

    // first offsets from reddit post
    int startOffset = 0x310;
    int endOffset = 0x28030f;

    for (int i = startOffset; i < endOffset; i++) {
        size_t data = savefile[i];
        std::string output;

        output = fmt::format(" {:02x}", data & 0xFF);

        // Check if we need to add a newline
        if (i % 56 == 0 && i != 0)
            output += "\n";

        fmt::print(output);
    }

    // null terminate stdout
    fmt::print("\n");
};

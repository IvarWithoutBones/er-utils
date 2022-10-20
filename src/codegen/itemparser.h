#include <fstream>
#include <string_view>
#include <vector>

/**
 * @brief Parse a CSV file from erdb into a C++ file containing an array.
 */
class ItemParser {
  private:
    struct Identifier {
        const std::string_view name;
        size_t index{};
    };

    Identifier nameIdentifier{"Row Name"};
    Identifier idIdentifier{"Row ID"};
    constexpr static char delimiter{';'};
    std::ifstream file;

    const std::vector<std::string> parseLine(std::string_view line) const;
    const std::string normalise(std::string_view string) const;

  public:
    void generate();

    ItemParser(const std::string_view path);
};

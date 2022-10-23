#include "itemparser.h"
#include "../util.h"
#include <array>
#include <fmt/core.h>
#include <sstream>

const std::vector<std::string> ItemParser::parseLine(std::string_view line) const {
    std::vector<std::string> result;
    std::stringstream stream(line.data());
    std::string item;
    while (std::getline(stream, item, delimiter))
        result.push_back(item);
    return result;
}

ItemParser::ItemParser(const std::string_view path) : file(path.data()) {
    std::string firstLine;
    std::getline(file, firstLine);
    auto columns{parseLine(firstLine)};
    for (size_t itr{}; itr < columns.size(); itr++) {
        const auto column{columns.at(itr)};
        if (column == nameIdentifier.name)
            nameIdentifier.index = itr;
        else if (column == idIdentifier.name)
            idIdentifier.index = itr;
    }
}

const std::string ItemParser::normalise(std::string_view string) const {
    constexpr std::array<unsigned char, 11> disallowed{'[', ']', '(', ')', '\'', '.', ',', '"', ':', '!', '&'};
    std::string result{};
    for (std::string::size_type itr{}; itr < string.size(); itr++) {
        const auto character{string.at(itr)};
        const auto previous{result.back()};
        if ((previous == '-' && character == '+') || (previous == '-' && character == ' ') || std::find(disallowed.begin(), disallowed.end(), character) != disallowed.end())
            continue;
        else if (character == ' ')
            result += '-';
        else
            result += static_cast<unsigned char>(std::tolower(character));
    }
    return result;
}

void ItemParser::generate() {
    std::vector<std::pair<std::string, std::string>> items;
    std::string line;

    while (std::getline(file, line)) {
        const auto columns{parseLine(line)};
        if (columns.size() < std::max(nameIdentifier.index, idIdentifier.index))
            continue;
        const auto name{normalise(columns.at(nameIdentifier.index))};
        const auto id{columns.at(idIdentifier.index)};
        if (name.empty() || id.empty())
            continue;

        // clang-format off
        if (std::find_if(items.begin(), items.end(), [name, id](const auto item) constexpr {
            return item.first == name || item.second == id;
        }) != items.end())
            continue;
        // clang-format on
        items.push_back({normalise(columns.at(nameIdentifier.index)), columns.at(idIdentifier.index)});
    }
    if (items.empty())
        throw exception("No items found while attempting to create generateditems.h");

    fmt::print("#pragma once\n"
               "#include <array>\n"
               "#include <string_view>\n\n"
               "namespace GeneratedItems {{\n\n"
               "struct Item {{\n"
               "    std::string_view name;\n"
               "    std::int32_t id;\n"
               "}};\n\n"
               "constexpr static std::array<Item, {}> items{{{{\n",
               items.size());
    for (auto item : items)
        fmt::print("    {{\"{}\", {}}},\n", item.first, item.second);
    fmt::print("}}}};\n\n"
               "}} // namespace GeneratedItems\n");
}

int main() {
    std::fstream versionFile{"external/erdb/latest_version.txt", std::ios::in};
    std::string version;
    std::getline(versionFile, version);
    ItemParser parser{fmt::format("external/erdb/gamedata/_Extracted/{}/EquipParamGoods.csv", version)};
    parser.generate();
}

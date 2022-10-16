#include "util.h"
#include <array>
#include <list>
#include <map>

namespace savepatcher {

constexpr static u8 ItemSize = 4;
constexpr static std::array<u8, 2> ItemDelimiter{0x0, 0xB0};

// Sequence in savefile: id groupId 00 B0 amount 00 00 00 ?? ?? 00 00
struct Item {
  public:
    u8 id{};
    u8 group{};
    std::array<u8, ItemSize> data;

    constexpr Item(u8 id, u8 groupId) : id{id}, group{groupId}, data{id, groupId, ItemDelimiter.front(), ItemDelimiter.back()} {}
    constexpr Item() : data{0, 0, ItemDelimiter.front(), ItemDelimiter.back()} {}
};

struct ItemResult {
    std::string_view name{};
    size_t offset;
    Item item;
    u32 quanity{};
    std::vector<size_t> duplicates{};

    ItemResult(ItemResult result, std::string_view name) : name{name}, offset{result.offset}, item{result.item} {}
    ItemResult(ItemResult result, std::string_view name, u32 quanity) : name{name}, offset{result.offset}, item{result.item}, quanity{quanity} {}
    ItemResult(ItemResult result, u32 quanity) : offset{result.offset}, item{result.item}, quanity{quanity} {}
    ItemResult(size_t offset, Item item) : offset{offset}, item{item} {}
    ItemResult(size_t offset, Item item, u32 quanity) : offset{offset}, item{item}, quanity{quanity} {}

    bool operator<(const ItemResult &rhs) {
        if (name.empty())
            return quanity < rhs.quanity;
        else
            return name < rhs.name;
    }

    void insertDuplicate(size_t pos) {
        duplicates.emplace_back(pos);
    }
};

struct ItemGroup {
    std::string_view name;
    u8 id;
    bool found{true};

    ItemGroup(std::string_view name, u8 id) : name{name}, id{id} {}
    ItemGroup(bool found) : found{found} {}
};

struct ItemGroups {
  private:
    // clang-format off
    std::list<ItemGroup> groups{
        {"Rune", 0xB},
        {"SmithingStone", 0x27},
        {"FowlFoot", 0x4},
        {"CraftingMaterial", 0x51},
        {"Glovewort", 0x2A},
        {"BeastBone", 0x3B}
    };
    // clang-format on

  public:
    auto operator[](std::string name) {
        const auto result{std::find_if(groups.begin(), groups.end(), [&name](const ItemGroup &v) { return v.name == name; })};
        if (result != groups.end())
            return *result;
        throw exception("Could not find item group {}", name);
    }

    ItemGroup find(ItemResult item) {
        const auto result{std::find_if(groups.begin(), groups.end(), [item](const ItemGroup &v) { return v.id == item.item.group; })};
        if (result != groups.end())
            return *result;
        return {false};
    }
};

// TODO: make this a struct rather than a pair. This kinda sucks.
using ItemList = std::map<std::string, Item>;

/**
 * @brief A map of items containting data that can be searched and replaced
 */
class Items : public ItemList {
  public:
    ItemGroups groups{};

  private:
    const Item item(ItemGroup group, u8 id) const {
        return Item{id, group.id};
    }

    void itemSequence(ItemGroup group, std::string name, u8 start, u8 amount) {
        for (u8 itr{}; itr < amount; itr++)
            this->try_emplace(fmt::format("{}-{}", name, itr + 1), Item(start + itr, group.id));
    }

    // clang-format off
    ItemList initialItems{
        {"lords-rune", item(groups["Rune"], 0x67)},
        {"gold-pickled-fowl-foot", item(groups["FowlFoot"], 0xB0)},
        {"silver-pickled-fowl-foot", item(groups["FowlFoot"], 0xA6)},
        {"thin-beast-bones", item(groups["BeastBone"], 0xEC)},
        {"hefty-beast-bone", item(groups["BeastBone"], 0xED)},
        {"ancient-dragon-smithing-stone", item(groups["SmithingStone"], 0x9C)},
        {"somber-ancient-dragon-smithing-stone", item(groups["SmithingStone"], 0xB8)},
        {"somber-smithing-stone-9", item(groups["SmithingStone"], 0xD8)}, // Not in the sequence of other somber stones
        {"gold-firefly", item(groups["CraftingMaterial"], 0x4B)},
        {"root-resin", item(groups["CraftingMaterial"], 0x27)},
        {"smoldering-butterfly", item(groups["CraftingMaterial"], 0x42)},
        {"mushroom", item(groups["CraftingMaterial"], 0x18)},
        {"melted-mushroom", item(groups["CraftingMaterial"], 0x19)},
        {"golden-centipede", item(groups["CraftingMaterial"], 0x54)},
        {"glintstone-firefly", item(groups["CraftingMaterial"], 0x4C)},
        {"cracked-crystal", item(groups["CraftingMaterial"], 0x2C)},
        {"volcanic-stone", item(groups["CraftingMaterial"], 0x72)},
        {"crystal-cave-moss", item(groups["CraftingMaterial"], 0x6A)},
        {"budding-cave-moss", item(groups["CraftingMaterial"], 0x69)},
        {"sanctuary-stone", item(groups["CraftingMaterial"], 0x3B)},

        // Unknown groups
        {"flight-pinion", Item(0xD4, 0x3A)},
        {"rune-arc", Item(0xBE, 0x0)},
        {"larval-tear", Item(0xF9, 0x1F)},
        {"rowa-fruit", Item(0xF0, 0x50)},
        {"throwing-dagger", Item(0xA4, 0x06)},
    };
    // clang-format on

    void goldenRunes() {
        itemSequence(groups["Rune"], "golden-rune", 0x54, 9);
    }

    void herosRunes() {
        itemSequence(groups["Rune"], "heros-rune", 0x5D, 5);
    }

    void smithingStones() {
        itemSequence(groups["SmithingStone"], "smithing-stone", 0x74, 8);
    }

    void somberSmithingStones() {
        itemSequence(groups["SmithingStone"], "somber-smithing-stone", 0xB0, 8);
    }

    void graveGloveworts() {
        itemSequence(groups["Glovewort"], "grave-glovewort", 0x94, 9);
    }

    void ghostGloveworts() {
        itemSequence(groups["Glovewort"], "ghost-glovewort", 0x9E, 9);
    }

  public:
    Items() {
        // TODO: Add these to the container automatically?
        goldenRunes();
        graveGloveworts();
        ghostGloveworts();
        smithingStones();
        herosRunes();
        somberSmithingStones();
        this->merge(initialItems);
    }

    const Item operator[](std::string_view name) {
        if (this->find(name.data()) != this->end())
            return this->at(name.data());
        throw exception("Unknown item '{}'", name);
    }

    const std::string findId(ItemResult item) {
        const auto result{std::find_if(this->begin(), this->end(), [item](const auto &v) { return v.second.id == item.item.id; })};
        if (result != this->end())
            return result->first;
        return {};
    }
};

} // namespace savepatcher

#include "util.h"
#include <array>
#include <list>
#include <map>

namespace Items {

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

    bool operator<(const ItemResult &rhs);

    void insertDuplicate(size_t pos);

    ItemResult(ItemResult result, std::string_view name, u32 quanity) : name{name}, offset{result.offset}, item{result.item}, quanity{quanity} {}
    ItemResult(ItemResult result, u32 quanity) : offset{result.offset}, item{result.item}, quanity{quanity} {}
    ItemResult(size_t offset, Item item) : offset{offset}, item{item} {}
};

struct ItemGroup {
    std::string_view name;
    u8 id;
    bool found{true};

    ItemGroup(std::string_view name, u8 id) : name{name}, id{id} {}
    ItemGroup(bool found) : found{found} {}
};

// TODO: make this a struct rather than a pair. This kinda sucks.
using ItemList = std::map<std::string, Item>;

/**
 * @brief A map of items containting data that can be searched and replaced
 */
class Items : public ItemList {
  private:
    // clang-format off
    std::list<ItemGroup> groups{
        {"Rune", 0xB},
        {"SmithingStone", 0x27},
        {"FowlFoot", 0x4},
        {"CraftingMaterial", 0x51},
        {"Glovewort", 0x2A},
        {"BeastBone", 0x3B},
    };

    ItemList items{
        // Crafting materials
        {"gold-firefly", item(group("CraftingMaterial"), 0x4B)},
        {"root-resin", item(group("CraftingMaterial"), 0x27)},
        {"smoldering-butterfly", item(group("CraftingMaterial"), 0x42)},
        {"mushroom", item(group("CraftingMaterial"), 0x18)},
        {"melted-mushroom", item(group("CraftingMaterial"), 0x19)},
        {"golden-centipede", item(group("CraftingMaterial"), 0x54)},
        {"glintstone-firefly", item(group("CraftingMaterial"), 0x4C)},
        {"cracked-crystal", item(group("CraftingMaterial"), 0x2C)},
        {"volcanic-stone", item(group("CraftingMaterial"), 0x72)},
        {"crystal-cave-moss", item(group("CraftingMaterial"), 0x6A)},
        {"budding-cave-moss", item(group("CraftingMaterial"), 0x69)},
        {"sanctuary-stone", item(group("CraftingMaterial"), 0x3B)},

        // Fowl feet
        {"gold-pickled-fowl-foot", item(group("FowlFoot"), 0xB0)},
        {"silver-pickled-fowl-foot", item(group("FowlFoot"), 0xA6)},

        // Beast bones
        {"thin-beast-bones", item(group("BeastBone"), 0xEC)},
        {"hefty-beast-bone", item(group("BeastBone"), 0xED)},

        // Smithing stones (not present in the sequences)
        {"ancient-dragon-smithing-stone", item(group("SmithingStone"), 0x9C)},
        {"somber-ancient-dragon-smithing-stone", item(group("SmithingStone"), 0xB8)},
        {"somber-smithing-stone-9", item(group("SmithingStone"), 0xD8)},

        // Miscellaneous
        {"lords-rune", item(group("Rune"), 0x67)},
        {"golden-seed", Item(0x1A, 0x27)},
        {"flight-pinion", Item(0xD4, 0x3A)},
        {"rune-arc", Item(0xBE, 0x0)},
        {"larval-tear", Item(0xF9, 0x1F)},
        {"rowa-fruit", Item(0xF0, 0x50)},
        {"throwing-dagger", Item(0xA4, 0x06)},
    };
    // clang-format on

    const Item item(ItemGroup group, u8 id) const;

    void sequence(std::string name, ItemGroup group, u8 startId, u8 amount);

    ItemGroup group(std::string_view name);

  public:
    Items() {
        sequence("golden-rune", group("Rune"), 0x54, 9);
        sequence("heros-rune", group("Rune"), 0x5D, 5);
        sequence("smithing-stone", group("SmithingStone"), 0x74, 8);
        sequence("somber-smithing-stone", group("SmithingStone"), 0xB0, 8);
        sequence("grave-glovewort", group("Glovewort"), 0x94, 9);
        sequence("ghost-glovewort", group("Glovewort"), 0x9E, 9);
        this->merge(items);
    }

    const Item operator[](std::string_view name);

    const std::string findId(ItemResult item);

    ItemGroup hasGroup(ItemResult item);

    void print() const;
};

} // namespace Items

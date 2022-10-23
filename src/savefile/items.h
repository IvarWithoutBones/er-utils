#include "../codegen/generateditems.h"
#include "../util.h"
#include <array>
#include <list>
#include <vector>
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
    // clang-format on

    ItemGroup group(std::string_view name);

  public:
    Items() {
        for (auto item : GeneratedItems::items) {
            Item i{static_cast<u8>(item.id & 0xff), static_cast<u8>(item.id >> 8)};
            this->emplace(item.name, i);
        }
    }

    const Item operator[](std::string_view name);

    const std::string findId(ItemResult item);

    ItemGroup hasGroup(ItemResult item);

    void print() const;
};

} // namespace Items

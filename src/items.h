#include "util.h"
#include <array>
#include <map>

namespace savepatcher {

struct Item {
  private:
    constexpr static std::array<u8, 2> delimiter{0x0, 0xB0};

  public:
    const std::array<u8, 4> data;
    constexpr Item(u8 id, u8 groupId) : data{id, groupId, delimiter.front(), delimiter.back()} {}
};

using ItemList = std::map<std::string, Item>;
using ItemGroup = u8;

/**
 * @brief A map of items containting data that can be searched and replaced
 */
class Items : public ItemList {
  private:
    constexpr static ItemGroup Rune{0xB};
    constexpr static ItemGroup SmithingStone{0x27};
    constexpr static ItemGroup FowlFoot{0x4};

    template <ItemGroup Group> const Item item(u8 id) const {
        return Item{id, Group};
    }

    template <ItemGroup Group, u8 amount> void itemSequence(std::string name, u8 start) {
        for (u8 itr{}; itr < amount; itr++)
            this->try_emplace(fmt::format("{}-{}", name, itr + 1), Item(start + itr, Group));
    }

    // clang-format off
    ItemList initialItems{
        {"lords-rune", item<Rune>(0x67)},
        {"flight-pinion", Item(0xD4, 0x3A)},
        {"rune-arc", Item(0xBE, 0x0)},
        {"larval-tear", Item(0xF9, 0x1F)},
        {"gold-firefly", Item(0x4B, 0x51)},
        {"root-resin", Item(0x27, 0x51)},
        {"gold-pickled-fowl-foot", item<FowlFoot>(0xB0)},
        {"silver-pickled-fowl-foot", item<FowlFoot>(0xA6)},
        {"ancient-dragon-smithing-stone", item<SmithingStone>(0x9C)},
        {"somber-ancient-dragon-smithing-stone", item<SmithingStone>(0xB8)},
    };
    // clang-format on

    void goldenRunes() {
        itemSequence<Rune, 12>("golden-rune", 0x54);
    }

    void smithingStones() {
        itemSequence<SmithingStone, 8>("smithing-stone", 0x74);
    }

    void somberSmithingStones() {
        itemSequence<SmithingStone, 7>("somber-smithing-stone", 0xB0);
        this->try_emplace("somber-smithing-stone-9", item<SmithingStone>(0xD8));
    }

  public:
    Items() {
        // TODO: Add these to the container automatically?
        goldenRunes();
        smithingStones();
        somberSmithingStones();
        this->merge(initialItems);
    }

    const Item operator[](std::string_view name) {
        if (this->find(name.data()) != this->end())
            return this->at(name.data());
        throw exception("Unknown item '{}'", name);
    }
};

} // namespace savepatcher

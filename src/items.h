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
    constexpr static ItemGroup CraftingMaterial{0x51};
    constexpr static ItemGroup Glovewort{0x2A};
    constexpr static ItemGroup BeastBone{0x3B};

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
        {"gold-pickled-fowl-foot", item<FowlFoot>(0xB0)},
        {"silver-pickled-fowl-foot", item<FowlFoot>(0xA6)},
        {"thin-beast-bones", item<BeastBone>(0xEC)},
        {"hefty-beast-bone", item<BeastBone>(0xED)},
        {"ancient-dragon-smithing-stone", item<SmithingStone>(0x9C)},
        {"somber-ancient-dragon-smithing-stone", item<SmithingStone>(0xB8)},
        {"somber-smithing-stone-9", item<SmithingStone>(0xD8)}, // Not in the sequence of other somber stones
        {"gold-firefly", item<CraftingMaterial>(0x4B)},
        {"root-resin", item<CraftingMaterial>(0x27)},
        {"smoldering-butterfly", item<CraftingMaterial>(0x42)},
        {"mushroom", item<CraftingMaterial>(0x18)},
        {"melted-mushroom", item<CraftingMaterial>(0x19)},
        {"golden-centipede", item<CraftingMaterial>(0x54)},
        {"flight-pinion", Item(0xD4, 0x3A)},
        {"rune-arc", Item(0xBE, 0x0)},
        {"larval-tear", Item(0xF9, 0x1F)},
        {"rowa-fruit", Item(0xF0, 0x50)},
        {"throwing-dagger", Item(0xA4, 0x06)},
    };
    // clang-format on

    void goldenRunes() {
        itemSequence<Rune, 12>("golden-rune", 0x54);
    }

    void smithingStones() {
        // TODO: smithing stone 8 is not in the sequence
        itemSequence<SmithingStone, 7>("smithing-stone", 0x74);
    }

    void somberSmithingStones() {
        itemSequence<SmithingStone, 9>("somber-smithing-stone", 0xB0);
    }

    void graveGloveworts() {
        itemSequence<Glovewort, 9>("grave-glovewort", 0x94);
    }

    void ghostGloveworts() {
        itemSequence<Glovewort, 9>("ghost-glovewort", 0x9E);
    }

  public:
    Items() {
        // TODO: Add these to the container automatically?
        goldenRunes();
        graveGloveworts();
        ghostGloveworts();
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

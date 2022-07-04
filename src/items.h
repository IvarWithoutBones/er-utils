#include "util.h"
#include <array>
#include <map>

namespace savepatcher {

struct Item {
  private:
    constexpr static std::array<u8, 2> delimiter{0x0, 0xB0};

  public:
    const std::array<u8, 4> data;

    constexpr Item(u8 groupId, u8 id) : data{id, groupId, delimiter.front(), delimiter.back()} {}
};

/**
 * @brief https://github.com/Ariescyn/EldenRing-Save-Manager/blob/main/itemdata.py
 */
class Items : public std::map<std::string, Item> {
  private:
    constexpr static size_t Rune{0xB};

    template <int Group> constexpr static Item makeItem(u8 id) {
        return Item{Group, id};
    }

    std::map<std::string, Item> staticItems{
        {"lords-rune", makeItem<Rune>(0x67)},
    };

    void makeGoldenRunes() {
        constexpr u8 goldenRuneCount{13};
        for (u8 i{1}; i <= goldenRuneCount; i++) {
            auto item{makeItem<Rune>(0x53 + i)};
            this->try_emplace(fmt::format("golden-rune-{}", i), item);
        }
    }

  public:
    Items() {
        makeGoldenRunes();
        for (auto item : staticItems)
            this->try_emplace(item.first, item.second);
    }

    const Item operator[](std::string_view name) {
        if (this->find(name.data()) != this->end())
            return this->at(name.data());
        throw exception("Unknown item '{}'", name);
    }
};

} // namespace savepatcher

#include "items.h"

namespace Items {

const Item Items::item(ItemGroup group, u8 id) const {
    return Item{id, group.id};
}

void Items::sequence(std::string name, ItemGroup group, u8 startId, u8 amount) {
    for (u8 itr{}; itr < amount; itr++)
        this->try_emplace(fmt::format("{}-{}", name, itr + 1), Item(startId + itr, group.id));
}

const std::string Items::findId(ItemResult item) {
    const auto result{std::find_if(this->begin(), this->end(), [item](const auto &v) {
        return v.second.id == item.item.id;
    })};
    if (result != this->end())
        return result->first;
    return {};
}

ItemGroup Items::hasGroup(ItemResult item) {
    const auto result{std::find_if(groups.begin(), groups.end(), [item](const ItemGroup &v) {
        return v.id == item.item.group;
    })};

    if (result != groups.end())
        return *result;
    return {false};
}

ItemGroup Items::group(std::string_view name) {
    const auto result{std::find_if(groups.begin(), groups.end(), [&name](const ItemGroup &v) {
        return v.name == name;
    })};

    if (result != groups.end())
        return *result;
    throw exception("Could not find item group {}", name);
}

void ItemResult::insertDuplicate(size_t pos) {
    duplicates.emplace_back(pos);
}

bool ItemResult::operator<(const ItemResult &rhs) {
    if (name.empty())
        return quanity < rhs.quanity;
    else
        return name < rhs.name;
}

const Item Items::operator[](std::string_view name) {
    if (this->find(name.data()) != this->end())
        return this->at(name.data());
    throw exception("Unknown item '{}'", name);
}

} // namespace Items

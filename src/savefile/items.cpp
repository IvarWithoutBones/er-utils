#include "items.h"

namespace Items {

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

void Items::print() const {
    for (auto item : *this)
        fmt::print("{}\n", item.first);
};

} // namespace Items

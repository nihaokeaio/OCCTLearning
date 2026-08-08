//
// Created by ZQD on 26-8-7.
//

#pragma once
#include "Data/ElementId.h"

enum class ValueRole
{
    User,
    DependencyGraph,
    Deserialization
};

struct PropertyAddress
{
    ElementId elementId;
    std::string propertyName;
    ValueRole valueRole = ValueRole::User;
    auto operator<=>(const PropertyAddress&) const = default;
};

// 特化 std::hash
template<>
struct std::hash<PropertyAddress> {
    std::size_t operator()(const PropertyAddress &addr) const noexcept {
        std::size_t h1 = std::hash<ElementId>{}(addr.elementId);
        std::size_t h2 = std::hash<std::string>{}(addr.propertyName);
        std::size_t h3 = std::hash<int>{}(static_cast<int>(addr.valueRole));

        // 经典的组合方式：移位 + 异或
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

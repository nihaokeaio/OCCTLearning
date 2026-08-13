//
// Created by ZQD on 26-8-7.
//

#pragma once
#include "Data/ElementId.h"

enum class ChangeSource
{
    User,
    DependencyGraph,
    Deserialization
};

struct PropertyAddress
{
    ElementId elementId;
    std::string propertyName;
    auto operator<=>(const PropertyAddress&) const = default;
};

struct PropertyChange
{
    ChangeSource changeSource;
    PropertyAddress address;
};

// 特化 std::hash
template<>
struct std::hash<PropertyAddress> {
    std::size_t operator()(const PropertyAddress &addr) const noexcept {
        std::size_t h1 = std::hash<ElementId>{}(addr.elementId);
        std::size_t h2 = std::hash<std::string>{}(addr.propertyName);
        // 经典的组合方式：移位 + 异或
        return h1 ^ (h2 << 1);
    }
};

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




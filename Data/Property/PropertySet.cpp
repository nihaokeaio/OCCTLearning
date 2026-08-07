//
// Created by ZQD on 2026/6/3.
//

#include "PropertySet.h"

PropertySet::PropertySet() = default;

PropertySet::~PropertySet() = default;

void PropertySet::Set(const std::string& key, const PropertyValue& value)
{
    m_Properties[key] = value;
}

std::optional<PropertyValue> PropertySet::Get(const std::string& key) const
{
    if (Exists(key))
    {
        return m_Properties.at(key);
    }
    return std::nullopt;
}

void PropertySet::Remove(const std::string& key)
{
    m_Properties.erase(key);
}

void PropertySet::Clear()
{
    m_Properties.clear();
}

bool PropertySet::Exists(const std::string& key) const
{
    return m_Properties.contains(key);
}

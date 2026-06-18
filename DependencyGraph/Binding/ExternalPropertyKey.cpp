#include "DependencyGraph/Binding/ExternalPropertyKey.h"

#include <functional>

std::string ExternalPropertyKey::ToString() const
{
    if (objectId.empty())
    {
        return propertyKey;
    }
    if (propertyKey.empty())
    {
        return objectId;
    }
    return objectId + "." + propertyKey;
}

bool ExternalPropertyKey::operator==(const ExternalPropertyKey& other) const
{
    return objectId == other.objectId && propertyKey == other.propertyKey;
}

size_t std::hash<ExternalPropertyKey>::operator()(const ExternalPropertyKey& key) const noexcept
{
    const auto objectHash = std::hash<std::string>{}(key.objectId);
    const auto propertyHash = std::hash<std::string>{}(key.propertyKey);
    return objectHash ^ (propertyHash + 0x9e3779b9 + (objectHash << 6) + (objectHash >> 2));
}

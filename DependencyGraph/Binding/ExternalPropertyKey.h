#pragma once

#include <cstddef>
#include <string>

struct ExternalPropertyKey
{
    std::string objectId;
    std::string propertyKey;

    [[nodiscard]] std::string ToString() const;
    bool operator==(const ExternalPropertyKey& other) const;
};

namespace std
{
    template <>
    struct hash<ExternalPropertyKey>
    {
        size_t operator()(const ExternalPropertyKey& key) const noexcept;
    };
}

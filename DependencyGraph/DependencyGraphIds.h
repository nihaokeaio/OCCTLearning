#pragma once

#include <cstddef>
#include <random>
#include <unordered_set>

class UniqueRandomGenerator
{
private:
    std::mt19937 rng;
    std::unordered_set<int> generated;
    int minValue;
    int maxValue;

public:
    UniqueRandomGenerator(int min, int max);

    int next();
    void reset();
    size_t generatedCount() const;
    bool hasNext() const;
};

struct ComputerNodeId
{
    ComputerNodeId();

    bool operator==(const ComputerNodeId& other) const;

    int m_Id = 0;
};

struct ValueId
{
    ValueId();

    bool operator==(const ValueId& other) const;

    int m_Id = 0;
};

namespace std
{
    template <>
    struct hash<ComputerNodeId>
    {
        size_t operator()(const ComputerNodeId& n) const noexcept;
    };

    template <>
    struct hash<ValueId>
    {
        size_t operator()(const ValueId& n) const noexcept;
    };
}

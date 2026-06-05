#include "DependencyGraphIds.h"

#include <chrono>
#include <stdexcept>

namespace
{
    UniqueRandomGenerator generator(1, 100);
}

UniqueRandomGenerator::UniqueRandomGenerator(int min, int max)
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      minValue(min),
      maxValue(max)
{
    if (max - min + 1 < 1)
    {
        throw std::runtime_error("范围太小，无法生成唯一值");
    }
}

int UniqueRandomGenerator::next()
{
    if (generated.size() >= (maxValue - minValue + 1))
    {
        throw std::runtime_error("已生成所有可能的值，没有更多不重复的值");
    }

    std::uniform_int_distribution<int> dist(minValue, maxValue);
    int value;
    do
    {
        value = dist(rng);
    }
    while (generated.find(value) != generated.end());

    generated.insert(value);
    return value;
}

void UniqueRandomGenerator::reset()
{
    generated.clear();
}

size_t UniqueRandomGenerator::generatedCount() const
{
    return generated.size();
}

bool UniqueRandomGenerator::hasNext() const
{
    return generated.size() < (maxValue - minValue + 1);
}

ComputerNodeId::ComputerNodeId()
{
    m_Id = generator.next();
}

bool ComputerNodeId::operator==(const ComputerNodeId& other) const
{
    return m_Id == other.m_Id;
}

ValueId::ValueId()
{
    m_Id = generator.next();
}

bool ValueId::operator==(const ValueId& other) const
{
    return m_Id == other.m_Id;
}

size_t std::hash<ComputerNodeId>::operator()(const ComputerNodeId& n) const noexcept
{
    return std::hash<int>()(n.m_Id);
}

size_t std::hash<ValueId>::operator()(const ValueId& n) const noexcept
{
    return std::hash<int>()(n.m_Id);
}

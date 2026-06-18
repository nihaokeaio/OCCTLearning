#include "DependencyGraph/Binding/ValueBindingRegistry.h"

#include <utility>

void ValueBindingRegistry::RegisterBinding(ValueId valueId, ExternalPropertyKey key)
{
    if (const auto oldKeyIt = m_ValueToExternal.find(valueId); oldKeyIt != m_ValueToExternal.end())
    {
        m_ExternalToValue.erase(oldKeyIt->second);
    }

    if (const auto oldValueIt = m_ExternalToValue.find(key); oldValueIt != m_ExternalToValue.end())
    {
        m_ValueToExternal.erase(oldValueIt->second);
    }

    m_ExternalToValue.emplace(key, valueId);
    m_ValueToExternal.emplace(valueId, std::move(key));
}

void ValueBindingRegistry::UnregisterBinding(ValueId valueId)
{
    const auto keyIt = m_ValueToExternal.find(valueId);
    if (keyIt == m_ValueToExternal.end())
    {
        return;
    }

    m_ExternalToValue.erase(keyIt->second);
    m_ValueToExternal.erase(keyIt);
}

void ValueBindingRegistry::Clear()
{
    m_ValueToExternal.clear();
    m_ExternalToValue.clear();
}

const ExternalPropertyKey* ValueBindingRegistry::FindExternalKey(ValueId valueId) const
{
    const auto it = m_ValueToExternal.find(valueId);
    if (it == m_ValueToExternal.end())
    {
        return nullptr;
    }
    return &it->second;
}

const ValueId* ValueBindingRegistry::FindValueId(const ExternalPropertyKey& key) const
{
    const auto it = m_ExternalToValue.find(key);
    if (it == m_ExternalToValue.end())
    {
        return nullptr;
    }
    return &it->second;
}

const std::unordered_map<ValueId, ExternalPropertyKey>& ValueBindingRegistry::ValueToExternal() const
{
    return m_ValueToExternal;
}

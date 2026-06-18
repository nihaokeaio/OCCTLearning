#pragma once

#include "DependencyGraph/Binding/ExternalPropertyKey.h"
#include "DependencyGraph/Core/DependencyGraphIds.h"

#include <unordered_map>

class ValueBindingRegistry
{
public:
    void RegisterBinding(ValueId valueId, ExternalPropertyKey key);
    void UnregisterBinding(ValueId valueId);
    void Clear();

    [[nodiscard]] const ExternalPropertyKey* FindExternalKey(ValueId valueId) const;
    [[nodiscard]] const ValueId* FindValueId(const ExternalPropertyKey& key) const;
    [[nodiscard]] const std::unordered_map<ValueId, ExternalPropertyKey>& ValueToExternal() const;

private:
    std::unordered_map<ValueId, ExternalPropertyKey> m_ValueToExternal;
    std::unordered_map<ExternalPropertyKey, ValueId> m_ExternalToValue;
};

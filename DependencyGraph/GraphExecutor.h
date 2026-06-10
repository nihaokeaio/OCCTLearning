#pragma once

#include "DependencyGraphIds.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct DGContext;

struct GraphExecutor
{
    void MarkDirty(ValueId id, const DGContext* context = nullptr);
    bool Evaluate(DGContext* context);

    std::queue<ValueId> dirtyQueue;
    std::unordered_set<ValueId> m_DirtyValues;
    std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;
};

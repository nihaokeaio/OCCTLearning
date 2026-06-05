#pragma once

#include "DependencyGraphIds.h"

#include <queue>
#include <unordered_map>
#include <vector>

struct DGContext;

struct GraphExecutor
{
    void MarkDirty(ValueId id);
    bool Evaluate(DGContext* context);

    std::queue<ValueId> dirtyQueue;
    std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;
};

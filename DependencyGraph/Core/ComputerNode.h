#pragma once

#include "DependencyGraphIds.h"

#include <functional>
#include <vector>

#include "ComputerView.h"


struct DGContext;

struct ComputerNode
{
    using ComputeFunc = std::function<void(const ComputerView& computerViews)>;

    ComputerNode() = default;
    ComputerNode(std::vector<ValueId> inputs, std::vector<ValueId> outputs, ComputeFunc computeFunc);

    virtual ~ComputerNode() = default;
    virtual void Evaluator(DGContext& context);

    ComputerNodeId m_Id;
    std::vector<ValueId> m_Inputs;
    std::vector<ValueId> m_Outputs;
    ComputeFunc computeFunc;
};

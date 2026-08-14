#pragma once


#include <functional>
#include <string>
#include <vector>

#include "ComputerView.h"
#include "Data/Property/PropertyAddress.h"


struct DGContext;

using ComputerNodeId = ElementId;

struct ComputerNode
{
    using ComputeFunc = std::function<void(const ComputerView& computerViews)>;
    ComputerNode(std::vector<PropertyAddress> inputs, std::vector<PropertyAddress> outputs, ComputeFunc computeFunc,
                 std::string debugName = {});

    virtual ~ComputerNode() = default;
    virtual void Evaluator(DGContext& context);

    ComputerNodeId m_Id;
    std::string m_DebugName;
    std::vector<PropertyAddress> m_Inputs;
    std::vector<PropertyAddress> m_Outputs;
    ComputeFunc computeFunc;
};

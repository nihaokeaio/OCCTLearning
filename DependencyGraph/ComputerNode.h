#pragma once

#include "DependencyGraphIds.h"

struct DGContext;

struct ComputerNode
{
    virtual ~ComputerNode() = default;
    virtual void Evaluator(DGContext& context) = 0;

    ComputerNodeId m_Id;
};

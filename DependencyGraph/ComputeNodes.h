#pragma once

#include "ComputerNode.h"
#include "ValueHandle.h"

#include <functional>
#include <memory>

struct PositionToLengthNode : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    PositionToLengthNode(const ValueId& inputA, const ValueId& inputB, const ValueId& output);

    void Evaluator(DGContext& context) override;

    ValueId m_InputA;
    ValueId m_InputB;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1, const SmartPtr& v2)> computeFunc;
};

struct LengthToRectArea : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    LengthToRectArea(const ValueId& inputA, const ValueId& inputB, const ValueId& output);

    void Evaluator(DGContext& context) override;

    ValueId m_InputA;
    ValueId m_InputB;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1, const SmartPtr& v2)> computeFunc;
};

struct LengthToCircleArea : ComputerNode
{
    using SmartPtr = std::shared_ptr<ValueHandle>;

    LengthToCircleArea(const ValueId& inputA, const ValueId& output);

    void Evaluator(DGContext& context) override;

    ValueId m_InputA;
    ValueId m_Output;
    std::function<void(const SmartPtr& v0, const SmartPtr& v1)> computeFunc;
};

struct LengthToRenderNode : ComputerNode
{
    explicit LengthToRenderNode(const ValueId& input);

    void Evaluator(DGContext& context) override;

    ValueId m_Input;
};

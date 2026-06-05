#include "ComputeNodes.h"

#include "DGContext.h"
#include "Render.h"

PositionToLengthNode::PositionToLengthNode(const ValueId& inputA, const ValueId& inputB, const ValueId& output):
    m_InputA(inputA),
    m_InputB(inputB),
    m_Output(output)
{
}

void PositionToLengthNode::Evaluator(DGContext& context)
{
    const auto& v0 = context.m_Values[m_InputA];
    const auto& v1 = context.m_Values[m_InputB];
    const auto& v2 = context.m_Values[m_Output];
    computeFunc(v0, v1, v2);
}

LengthToRectArea::LengthToRectArea(const ValueId& inputA, const ValueId& inputB, const ValueId& output):
    m_InputA(inputA),
    m_InputB(inputB),
    m_Output(output)
{
}

void LengthToRectArea::Evaluator(DGContext& context)
{
    const auto& v0 = context.m_Values[m_InputA];
    const auto& v1 = context.m_Values[m_InputB];
    const auto& v2 = context.m_Values[m_Output];
    computeFunc(v0, v1, v2);
}

LengthToCircleArea::LengthToCircleArea(const ValueId& inputA, const ValueId& output):
    m_InputA(inputA),
    m_Output(output)
{
}

void LengthToCircleArea::Evaluator(DGContext& context)
{
    const auto& v0 = context.m_Values[m_InputA];
    const auto& v1 = context.m_Values[m_Output];
    computeFunc(v0, v1);
}

LengthToRenderNode::LengthToRenderNode(const ValueId& input):
    m_Input(input)
{
}

void LengthToRenderNode::Evaluator(DGContext& context)
{
    context.render->Update(m_Input);
}

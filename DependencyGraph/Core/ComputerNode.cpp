#include "ComputerNode.h"

#include "ComputerView.h"

#include <stdexcept>
#include <utility>

ComputerNode::ComputerNode(std::vector<ValueId> inputs, std::vector<ValueId> outputs, ComputeFunc computeFunc):
    m_Inputs(std::move(inputs)),
    m_Outputs(std::move(outputs)),
    computeFunc(std::move(computeFunc))
{
}

void ComputerNode::Evaluator(DGContext& context)
{
    if (!computeFunc)
    {
        throw std::runtime_error("ComputerNode has no compute function");
    }

    const ComputerView view(context, m_Inputs, m_Outputs);
    computeFunc(view);
}

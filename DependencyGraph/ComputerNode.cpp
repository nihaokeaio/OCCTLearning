#include "ComputerNode.h"

#include "ComputerView.h"

#include <stdexcept>
#include <utility>

#include "DGContext.h"
#include "Data/Document.h"

ComputerNode::ComputerNode(std::vector<PropertyAddress> inputs, std::vector<PropertyAddress> outputs,
                           ComputeFunc computeFunc):
    m_Id(Document::NewElementId()),
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

    const ComputerView view(&context, m_Inputs, m_Outputs);
    computeFunc(view);
}

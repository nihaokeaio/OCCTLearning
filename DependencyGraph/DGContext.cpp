#include "DGContext.h"

#include <utility>

#include "ComputerView.h"

DGContext::DGContext()
{
    m_GraphExecutor = std::make_unique<GraphExecutor>();
}

ComputerNode* DGContext::GetComputerNode(const ComputerNodeId& id)
{
    const auto iter = m_Nodes.find(id);
    if (iter != m_Nodes.end())
    {
        return iter->second.get();
    }
    return nullptr;
}

ValueHandle& DGContext::GetValueHandle(const ValueId& id)
{
    return *m_Values.at(id);
}

const ValueHandle& DGContext::GetValueHandle(const ValueId& id) const
{
    return *m_Values.at(id);
}

void DGContext::AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<ValueId>& inputs,
                                const std::vector<ValueId>& outputs)
{
    node->m_Inputs = inputs;
    for (const auto& inputId : inputs)
    {
        m_GraphExecutor->dependNodeLists[inputId].push_back(node->m_Id);
    }

    node->m_Outputs = outputs;
    for (const auto& outputId : outputs)
    {
        m_GraphExecutor->nodeOutputs[node->m_Id].push_back(outputId);
    }
    m_Nodes.insert({node->m_Id, std::move(node)});
}

ComputerNode* DGContext::AddComputeNode(const std::vector<ValueId>& inputs,
                                        const std::vector<ValueId>& outputs,
                                        ComputerNode::ComputeFunc computeFunc)
{
    auto node = std::make_unique<ComputerNode>();
    node->computeFunc = std::move(computeFunc);
    const auto nodePtr = node.get();
    AddComputerNode(std::move(node), inputs, outputs);
    return nodePtr;
}

ValueId DGContext::AddValueHandle(std::unique_ptr<ValueHandle>&& valueHandle)
{
    const auto id = valueHandle->m_Id;
    m_Values.insert_or_assign(id, std::move(valueHandle));
    return id;
}

void DGContext::Evaluator()
{
    m_GraphExecutor->Evaluate(this);
}

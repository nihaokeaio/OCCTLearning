#include "DGContext.h"

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

void DGContext::AddComputerNode(std::unique_ptr<ComputerNode>&& node,
                                const std::vector<std::shared_ptr<ValueHandle>>& inputs,
                                const std::vector<std::shared_ptr<ValueHandle>>& outputs)
{
    for (const auto& input : inputs)
    {
        m_GraphExecutor->dependNodeLists[input->m_Id].push_back(node->m_Id);
    }

    for (const auto& output : outputs)
    {
        m_GraphExecutor->nodeOutputs[node->m_Id].push_back(output->m_Id);
    }
    m_Nodes.insert({node->m_Id, std::move(node)});
}

void DGContext::AddValueHandle(const std::shared_ptr<ValueHandle>& valueHandle)
{
    m_Values.insert_or_assign(valueHandle->m_Id, valueHandle);
}

void DGContext::Evaluator()
{
    m_GraphExecutor->Evaluate(this);
}

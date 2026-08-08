#include "ComputerView.h"
#include "DependencyGraph//DGContext.h"

#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>


DGContext::DGContext(Document *document) : m_Document(document) {
    m_GraphExecutor = std::make_unique<GraphExecutor>();
}

bool DGContext::AddValueAddress(const PropertyAddress& propertyAddress)
{
    auto [iter,insert] = m_Values.insert(propertyAddress);
    return insert;
}

bool DGContext::RemoveValueAddress(const PropertyAddress& propertyAddress)
{
    if (!HasValue(propertyAddress)) {
        return false;
    }

    const auto dependentNodes = m_GraphExecutor->FindDependentNodes(propertyAddress);
    if (!dependentNodes.empty()) {
        throw std::runtime_error("Cannot remove value with dependent compute nodes");
    }
    if (m_GraphExecutor->FindProducerNode(propertyAddress) != nullptr)
    {
        throw std::runtime_error("Cannot remove value produced by a compute node: ");
    }

    m_GraphExecutor->RemoveValueAddress(propertyAddress);
    m_Values.erase(propertyAddress);
    return true;
}

bool DGContext::HasValue(const PropertyAddress& id) const
{
    return m_Values.contains(id);
}

ComputerNodeId DGContext::AddComputeNode(std::vector<PropertyAddress> inputs, std::vector<PropertyAddress> outputs,
                                         ComputerNode::ComputeFunc computeFunc)
{
    auto node = std::make_unique<ComputerNode>(inputs, outputs, computeFunc);
    return AddComputerNode(std::move(node), inputs, outputs);
}

ComputerNode* DGContext::GetComputerNode(const ComputerNodeId& id)
{
    const auto iter = m_Nodes.find(id);
    if (iter == m_Nodes.end())
    {
        return nullptr;
    }
    return iter->second.get();
}

bool DGContext::HasComputeNodeNode(const ComputerNodeId &id) const {
    return m_Nodes.contains(id);
}

void DGContext::MarkDirty(PropertyAddress node) const {
    m_GraphExecutor->MarkDirty(std::move(node));
}

bool DGContext::RemoveComputeNode(const ComputerNodeId &nodeId) {
    const auto iter = m_Nodes.find(nodeId);
    if (iter == m_Nodes.end()) {
        return false;
    }

    auto outputs = iter->second->m_Outputs;
    m_GraphExecutor->RemoveComputerNode(nodeId);
    m_Nodes.erase(iter);

    for (auto &outputId: outputs) {
        if (m_Values.contains(outputId) && m_GraphExecutor->FindProducerNode(outputId) == nullptr) {
            SetValueRole(outputId, ValueRole::User);
        }
    }
    return true;
}


ComputerNodeId DGContext::AddComputerNode(std::unique_ptr<ComputerNode>&& node, std::span<PropertyAddress> inputs,
                                          std::span<PropertyAddress> outputs)
{
    for (const auto& inputId : inputs)
    {
        if (!m_Values.contains(inputId))
        {
            throw std::runtime_error("Input value does not exist: " + std::to_string(inputId.elementId.GetValue()));
        }
    }
    std::unordered_set<PropertyAddress> uniqueOutputs;
    for (const auto& outputId : outputs)
    {
        if (!m_Values.contains(outputId))
        {
            throw std::runtime_error("Output value does not exist: " + std::to_string(outputId.elementId.GetValue()));
        }
        if (!uniqueOutputs.insert(outputId).second)
        {
            throw std::runtime_error("Duplicate output value in compute node");
        }
        if (m_GraphExecutor->FindProducerNode(outputId) != nullptr)
        {
            throw std::runtime_error("Output value already has a producer");
        }
    }

    if (WouldCreateCycle(inputs, outputs))
    {
        throw std::runtime_error("Dependency cycle detected while adding node ");
    }

    for (const auto& inputId : inputs)
    {
        m_GraphExecutor->AddDependentNode(inputId, node->m_Id);
    }

    for (auto &outputId: outputs) {
        SetValueRole(outputId, ValueRole::DependencyGraph);
        m_GraphExecutor->AddNodeOutput(node->m_Id, outputId);
    }
    const auto id = node->m_Id;
    m_Nodes.emplace(id, std::move(node));
    return id;
}

bool DGContext::CanReachValue(const PropertyAddress &from, const PropertyAddress &target) const {
    if (from == target)
    {
        return true;
    }

    std::queue<PropertyAddress> pending;
    std::unordered_set<PropertyAddress> visited;
    pending.push(from);

    while (!pending.empty())
    {
        const auto current = pending.front();
        pending.pop();

        if (!visited.insert(current).second)
        {
            continue;
        }
        for (const auto &nodeId: m_GraphExecutor->FindDependentNodes(current)) {
            for (const auto &outputId: m_GraphExecutor->FindNodeOutputs(nodeId)) {
                if (outputId == target)
                {
                    return true;
                }
                pending.push(outputId);
            }
        }
    }

    return false;
}

bool DGContext::WouldCreateCycle(std::span<PropertyAddress> inputs, std::span<PropertyAddress> outputs) const
{
    for (const auto& outputId : outputs)
    {
        for (const auto& inputId : inputs)
        {
            if (CanReachValue(outputId, inputId))
            {
                return true;
            }
        }
    }
    return false;
}

void DGContext::SetValueRole(PropertyAddress& valueAddress, const ValueRole role)
{
    valueAddress.valueRole = role;
}

ValueRole DGContext::GetValueRole(const PropertyAddress& valueAddress)
{
    return valueAddress.valueRole;
}




void DGContext::Clear()
{
    m_Values.clear();
    m_Nodes.clear();
    m_GraphExecutor->Clear();
}

DGContext::EvaluationResult DGContext::Evaluate()
{
    return m_GraphExecutor->Evaluate(this);
}

Document *DGContext::GetDocument() const {
    return m_Document;;
}

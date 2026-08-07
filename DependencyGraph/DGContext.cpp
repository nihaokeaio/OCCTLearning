#include "DependencyGraph//DGContext.h"

#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "ComputerView.h"

DGContext::DGContext()
{
    m_GraphExecutor = std::make_unique<GraphExecutor>();
}

bool DGContext::AddValueAddress(const PropertyAddress& propertyAddress)
{
    auto [iter,insert] = m_Values.insert(propertyAddress);
    return insert;
}

bool DGContext::RemoveValueAddress(const PropertyAddress& propertyAddress)
{
    if (!m_Values.contains(propertyAddress))
    {
        return false;
    }

    const auto dependentNodes = m_GraphExecutor->FindDependentNodes(propertyAddress);
    if (dependentNodes != nullptr && !dependentNodes->empty())
    {
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

ComputerNodeId DGContext::AddComputeNode(std::span<PropertyAddress> inputs, std::span<PropertyAddress> outputs,
                                         ComputerNode::ComputeFunc computeFunc)
{
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

bool DGContext::HasValue(const ValueId& id) const
{
    return m_Values.contains(id);
}

bool DGContext::HasNode(const ComputerNodeId& id) const
{
    return m_Nodes.contains(id);
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
        throw std::runtime_error("Dependency cycle detected while adding node " + NodeLabel(node->m_Id));
    }

    node->m_Inputs = inputs;
    for (const auto& inputId : inputs)
    {
        m_GraphExecutor->AddDependentNode(inputId, node->m_Id);
    }

    node->m_Outputs = outputs;
    for (const auto& outputId : outputs)
    {
        SetValueRole(outputId, ValueRole::Derived);
        m_GraphExecutor->AddNodeOutput(node->m_Id, outputId);
    }
    const auto id = node->m_Id;
    m_Nodes.insert({id, std::move(node)});
    return id;
}

ComputerNodeId DGContext::AddComputeNode(const std::vector<ValueId>& inputs,
                                         const std::vector<ValueId>& outputs,
                                         ComputerNode::ComputeFunc computeFunc)
{
    auto node = std::make_unique<ComputerNode>();
    node->computeFunc = std::move(computeFunc);
    return AddComputerNode(std::move(node), inputs, outputs);
}

bool DGContext::CanReachValue(PropertyAddress from, PropertyAddress target) const
{
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
        const auto dependentNodes = m_GraphExecutor->FindDependentNodes(current);
        if (dependentNodes == nullptr)
            continue;

        for (const auto& nodeId : dependentNodes)
        {
            const auto outputs = m_GraphExecutor->FindNodeOutputs(nodeId);
            if (outputs == nullptr)
                continue;

            for (const auto& outputId : outputs)
            {
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


bool DGContext::RemoveComputeNode(const ComputerNodeId& nodeId)
{
    const auto iter = m_Nodes.find(nodeId);
    if (iter == m_Nodes.end())
    {
        return false;
    }

    auto outputs = iter->second->m_Outputs;
    m_GraphExecutor->RemoveComputerNode(nodeId);
    m_Nodes.erase(iter);

    for (auto& outputId : outputs)
    {
        if (m_Values.contains(outputId) && m_GraphExecutor->FindProducerNode(outputId) == nullptr)
        {
            SetValueRole(outputId, ValueRole::User);
        }
    }
    return true;
}

bool DGContext::HasComputeNodeNode(const ComputerNodeId& id) const
{
    return m_Nodes.contains(id);
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
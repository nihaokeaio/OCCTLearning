#include "DGContext.h"

#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "ComputerView.h"

DGContext::DGContext()
{
    m_GraphExecutor = std::make_unique<GraphExecutor>();
}

ComputerNode* DGContext::GetComputerNode(const ComputerNodeId& id)
{
    return FindNode(id);
}

bool DGContext::HasValue(const ValueId& id) const
{
    return m_Values.contains(id);
}

bool DGContext::HasNode(const ComputerNodeId& id) const
{
    return m_Nodes.contains(id);
}

ValueHandle* DGContext::FindValueHandle(const ValueId& id)
{
    const auto iter = m_Values.find(id);
    if (iter == m_Values.end())
    {
        return nullptr;
    }
    return iter->second.get();
}

const ValueHandle* DGContext::FindValueHandle(const ValueId& id) const
{
    const auto iter = m_Values.find(id);
    if (iter == m_Values.end())
    {
        return nullptr;
    }
    return iter->second.get();
}

ComputerNode* DGContext::FindNode(const ComputerNodeId& nodeId)
{
    const auto iter = m_Nodes.find(nodeId);
    if (iter == m_Nodes.end())
    {
        return nullptr;
    }
    return iter->second.get();
}

const ComputerNode* DGContext::FindNode(const ComputerNodeId& nodeId) const
{
    const auto iter = m_Nodes.find(nodeId);
    if (iter == m_Nodes.end())
    {
        return nullptr;
    }
    return iter->second.get();
}

ValueHandle& DGContext::GetValueHandle(const ValueId& id)
{
    return *m_Values.at(id);
}

const ValueHandle& DGContext::GetValueHandle(const ValueId& id) const
{
    return *m_Values.at(id);
}

ComputerNode* DGContext::GetNode(const ComputerNodeId& nodeId)
{
    return m_Nodes.at(nodeId).get();
}

ComputerNodeId DGContext::AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<ValueId>& inputs,
                                          const std::vector<ValueId>& outputs)
{
    for (const auto& inputId : inputs)
    {
        if (!m_Values.contains(inputId))
        {
            throw std::runtime_error("Input value does not exist: " + std::to_string(inputId.m_Id));
        }
    }
    std::unordered_set<ValueId> uniqueOutputs;
    for (const auto& outputId : outputs)
    {
        if (!m_Values.contains(outputId))
        {
            throw std::runtime_error("Output value does not exist: " + std::to_string(outputId.m_Id));
        }
        if (!uniqueOutputs.insert(outputId).second)
        {
            throw std::runtime_error("Duplicate output value in compute node: " + ValueLabel(outputId));
        }
        if (m_GraphExecutor->FindProducerNode(outputId) != nullptr)
        {
            throw std::runtime_error("Output value already has a producer: " + ValueLabel(outputId));
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

bool DGContext::CanReachValue(ValueId from, ValueId target) const
{
    if (from == target)
    {
        return true;
    }

    std::queue<ValueId> pending;
    std::unordered_set<ValueId> visited;
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

        for (const auto& nodeId : *dependentNodes)
        {
            const auto outputs = m_GraphExecutor->FindNodeOutputs(nodeId);
            if (outputs == nullptr)
                continue;

            for (const auto& outputId : *outputs)
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

bool DGContext::WouldCreateCycle(const std::vector<ValueId>& inputs, const std::vector<ValueId>& outputs) const
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

ValueId DGContext::AddValueHandle(std::unique_ptr<ValueHandle>&& valueHandle)
{
    const auto id = valueHandle->m_Id;
    m_Values.insert_or_assign(id, std::move(valueHandle));
    return id;
}

std::string DGContext::DumpGraph() const
{
    std::ostringstream out;
    DumpGraph(out);
    return out.str();
}

void DGContext::DumpGraph(std::ostream& out) const
{
    out << "DependencyGraph\n";
    out << "Values:\n";
    for (const auto& [id, value] : m_Values)
    {
        out << "  " << ValueLabel(id) << " (" << ToString(value->m_Role) << ", id=" << id.m_Id << ")\n";
    }

    out << "Nodes:\n";
    for (const auto& [id, node] : m_Nodes)
    {
        out << "  " << NodeLabel(id) << " (id=" << id.m_Id << ")\n";

        out << "    inputs: ";
        for (size_t i = 0; i < node->m_Inputs.size(); ++i)
        {
            if (i != 0)
            {
                out << ", ";
            }
            out << ValueLabel(node->m_Inputs[i]);
        }
        out << "\n";

        out << "    outputs: ";
        for (size_t i = 0; i < node->m_Outputs.size(); ++i)
        {
            if (i != 0)
            {
                out << ", ";
            }
            out << ValueLabel(node->m_Outputs[i]);
        }
        out << "\n";
    }

    out << "Edges:\n";
    for (const auto& [nodeId, node] : m_Nodes)
    {
        for (const auto& inputId : node->m_Inputs)
        {
            out << "  " << ValueLabel(inputId) << " -> " << NodeLabel(nodeId) << "\n";
        }
        for (const auto& outputId : node->m_Outputs)
        {
            out << "  " << NodeLabel(nodeId) << " -> " << ValueLabel(outputId) << "\n";
        }
    }
}

std::string DGContext::ValueLabel(const ValueId& id) const
{
    const auto iter = m_ValueDebugNames.find(id);
    if (iter != m_ValueDebugNames.end() && !iter->second.empty())
    {
        return iter->second;
    }
    return std::string("Value#") + std::to_string(id.m_Id);
}

std::string DGContext::NodeLabel(const ComputerNodeId& id) const
{
    const auto iter = m_NodeDebugNames.find(id);
    if (iter != m_NodeDebugNames.end() && !iter->second.empty())
    {
        return iter->second;
    }
    return std::string("Node#") + std::to_string(id.m_Id);
}

ValueRole DGContext::GetValueRole(const ValueId& id) const
{
    return m_Values.at(id)->m_Role;
}

void DGContext::SetValueRole(const ValueId& id, ValueRole role)
{
    m_Values.at(id)->m_Role = role;
}

void DGContext::SetDebugName(const ValueId& id, std::string name)
{
    m_ValueDebugNames.insert_or_assign(id, std::move(name));
}

void DGContext::SetDebugName(const ComputerNodeId& id, std::string name)
{
    m_NodeDebugNames.insert_or_assign(id, std::move(name));
}

bool DGContext::RemoveValue(const ValueId& valueId)
{
    if (!m_Values.contains(valueId))
    {
        return false;
    }

    const auto dependentNodes = m_GraphExecutor->FindDependentNodes(valueId);
    if (dependentNodes != nullptr && !dependentNodes->empty())
    {
        throw std::runtime_error("Cannot remove value with dependent compute nodes: " + ValueLabel(valueId));
    }
    if (m_GraphExecutor->FindProducerNode(valueId) != nullptr)
    {
        throw std::runtime_error("Cannot remove value produced by a compute node: " + ValueLabel(valueId));
    }

    m_GraphExecutor->RemoveValue(valueId);
    m_ValueDebugNames.erase(valueId);
    m_Values.erase(valueId);
    return true;
}

bool DGContext::RemoveComputeNode(const ComputerNodeId& nodeId)
{
    const auto iter = m_Nodes.find(nodeId);
    if (iter == m_Nodes.end())
    {
        return false;
    }

    const auto outputs = iter->second->m_Outputs;
    m_GraphExecutor->RemoveNode(nodeId);
    m_NodeDebugNames.erase(nodeId);
    m_Nodes.erase(iter);

    for (const auto& outputId : outputs)
    {
        if (m_Values.contains(outputId) && m_GraphExecutor->FindProducerNode(outputId) == nullptr)
        {
            SetValueRole(outputId, ValueRole::UserInput);
        }
    }
    return true;
}

void DGContext::Clear()
{
    m_Values.clear();
    m_Nodes.clear();
    m_ValueDebugNames.clear();
    m_NodeDebugNames.clear();
    m_GraphExecutor->Clear();
}

DGContext::EvaluationResult DGContext::Evaluate()
{
    return m_GraphExecutor->Evaluate(this);
}

DGContext::EvaluationResult DGContext::Evaluator()
{
    return Evaluate();
}

void DGContext::SetTraceEnabled(bool enabled)
{
    m_GraphExecutor->SetTraceEnabled(enabled);
}

bool DGContext::IsTraceEnabled() const
{
    return m_GraphExecutor->IsTraceEnabled();
}

void DGContext::SetFlowTraceCallback(GraphExecutor::FlowTraceCallback callback)
{
    m_GraphExecutor->SetFlowTraceCallback(std::move(callback));
}

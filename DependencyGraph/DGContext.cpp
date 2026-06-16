#include "DGContext.h"

#include <queue>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "ComputerView.h"

DGContext::DGContext()
{
    m_GraphExecutor = std::make_unique<GraphExecutor>();
    m_Render = std::make_unique<Render>();
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

ComputerNode* DGContext::GetNode(const ComputerNodeId& nodeId)
{
    return m_Nodes.at(nodeId).get();
}

Render* DGContext::GetRender()
{
    return m_Render.get();
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
    for (const auto& outputId : outputs)
    {
        if (!m_Values.contains(outputId))
        {
            throw std::runtime_error("Output value does not exist: " + std::to_string(outputId.m_Id));
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
    for (const auto& id : m_Values | std::views::keys)
    {
        out << "  " << ValueLabel(id) << " (id=" << id.m_Id << ")\n";
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

void DGContext::SetDebugName(const ValueId& id, std::string name)
{
    m_ValueDebugNames.insert_or_assign(id, std::move(name));
}

void DGContext::SetDebugName(const ComputerNodeId& id, std::string name)
{
    m_NodeDebugNames.insert_or_assign(id, std::move(name));
}

bool DGContext::Evaluator()
{
    return m_GraphExecutor->Evaluate(this);
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

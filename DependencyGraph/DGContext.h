#pragma once

#include "ComputerNode.h"
#include "GraphExecutor.h"
#include "Render.h"
#include "ValueHandle.h"

#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct DGContext
{
    DGContext();

    [[nodiscard]] ComputerNode* GetComputerNode(const ComputerNodeId& id);

    ValueHandle& GetValueHandle(const ValueId& id);
    [[nodiscard]] const ValueHandle& GetValueHandle(const ValueId& id) const;

    [[nodiscard]] ComputerNode* GetNode(const ComputerNodeId& nodeId);

    [[nodiscard]] Render* GetRender();


    ComputerNodeId AddComputeNode(const std::vector<ValueId>& inputs,
                                  const std::vector<ValueId>& outputs,
                                  ComputerNode::ComputeFunc computeFunc);


    template <class T>
    ValueId CreateValue(const std::string& propertyName, const T& initialValue);

    template <class T>
    void SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value);


    bool Evaluator();
    void SetTraceEnabled(bool enabled);
    [[nodiscard]] bool IsTraceEnabled() const;
    void SetFlowTraceCallback(GraphExecutor::FlowTraceCallback callback);

public:
    [[nodiscard]] std::string DumpGraph() const;
    void DumpGraph(std::ostream& out) const;
    [[nodiscard]] std::string ValueLabel(const ValueId& id) const;
    [[nodiscard]] std::string NodeLabel(const ComputerNodeId& id) const;
    void SetDebugName(const ValueId& id, std::string name);
    void SetDebugName(const ComputerNodeId& id, std::string name);

private:
    ComputerNodeId AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<ValueId>& inputs,
                                   const std::vector<ValueId>& outputs);

    /// 环检测：沿 Value -> ComputerNode -> Value 方向判断可达性。
    bool CanReachValue(ValueId from, ValueId target) const;

    bool WouldCreateCycle(const std::vector<ValueId>& inputs, const std::vector<ValueId>& outputs) const;


    ValueId AddValueHandle(std::unique_ptr<ValueHandle>&& valueHandle);

private:
    std::unordered_map<ValueId, std::unique_ptr<ValueHandle>> m_Values;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unordered_map<ValueId, std::string> m_ValueDebugNames;
    std::unordered_map<ComputerNodeId, std::string> m_NodeDebugNames;
    std::unique_ptr<Render> m_Render;
    std::unique_ptr<GraphExecutor> m_GraphExecutor;
};

template <class T>
ValueId DGContext::CreateValue(const std::string& propertyName, const T& initialValue)
{
    auto valueHandle = std::make_unique<ValueHandle>();
    valueHandle->AddProperty(propertyName, initialValue);
    return AddValueHandle(std::move(valueHandle));
}

template <class T>
void DGContext::SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value)
{
    const auto iter = m_Values.find(valueId);
    if (iter == m_Values.end())
    {
        throw std::runtime_error("ValueHandle does not exist");
    }
    iter->second->SetProperty(propertyName, value);
    m_GraphExecutor->MarkDirty(valueId, this);
}

#pragma once

#include "ComputerNode.h"
#include "GraphExecutor.h"
#include <memory>
#include <unordered_map>


struct DGContext
{
    using EvaluationResult = GraphExecutor::EvaluationResult;

    DGContext();

    bool AddValueAddress(const PropertyAddress& propertyAddress);
    bool RemoveValueAddress(const PropertyAddress& propertyAddress);
    [[nodiscard]] bool HasValue(const PropertyAddress& id) const;

    ComputerNodeId AddComputeNode(std::span<PropertyAddress> inputs,
                                  std::span<PropertyAddress> outputs,
                                  ComputerNode::ComputeFunc computeFunc);

    [[nodiscard]] ComputerNode* GetComputerNode(const ComputerNodeId& id);
    [[nodiscard]] bool RemoveComputeNode(const ComputerNodeId& nodeId);
    [[nodiscard]] bool HasComputeNodeNode(const ComputerNodeId& id) const;


    void Clear();
    EvaluationResult Evaluate();


private:
    ComputerNodeId AddComputerNode(std::unique_ptr<ComputerNode>&& node, std::span<PropertyAddress> inputs,
                                   std::span<PropertyAddress> outputs);

    /// 环检测：沿 Value -> ComputerNode -> Value 方向判断可达性。
    bool CanReachValue(PropertyAddress from, PropertyAddress target) const;

    bool WouldCreateCycle(std::span<PropertyAddress> inputs, std::span<PropertyAddress> outputs) const;

    static void SetValueRole(PropertyAddress& valueAddress, ValueRole role);
    static ValueRole GetValueRole(const PropertyAddress& valueAddress);

private:
    std::unordered_set<PropertyAddress> m_Values;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unique_ptr<GraphExecutor> m_GraphExecutor;
};

#pragma once

#include "DependencyGraphIds.h"

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ComputerNode.h"
#include "Data/Property/PropertyAddress.h"

struct DGContext;

struct GraphExecutor
{

    struct EvaluationResult
    {
        bool evaluated = false;
        std::unordered_set<PropertyAddress> changedValues;
    };

    void MarkDirty(const PropertyAddress& address);
    EvaluationResult Evaluate(DGContext* context);

    void AddDependentNode(const PropertyAddress& address, ComputerNodeId nodeId);
    void AddNodeOutput(ComputerNodeId nodeId, const PropertyAddress& address);
    void RemoveComputerNode(ComputerNodeId nodeId);
    void RemoveValueAddress(const PropertyAddress& address);
    void Clear();

    [[nodiscard]] std::span<const ComputerNodeId> FindDependentNodes(const PropertyAddress& address) const;
    [[nodiscard]] std::span<const PropertyAddress> FindNodeOutputs(ComputerNodeId nodeId) const;
    [[nodiscard]] const ComputerNodeId* FindProducerNode(const PropertyAddress& address) const;

private:
    struct FlowEvent
    {
        PropertyAddress trigger;
        ComputerNodeId node;
        std::vector<PropertyAddress> outputs;
    };

    // 消费当前 dirty value 队列，收集本批需要执行的计算节点。
    // 注意：这里不会执行节点，只是把“哪些节点被脏值触发”转成一个 batch。
    void CollectDirtyNodes(std::queue<ComputerNodeId>& dirtyNodeQueue,
                           std::unordered_set<ComputerNodeId>& dirtyNodeIds);
    // 按已经排好的 batch 顺序执行节点，并把输出 value 继续标脏，驱动下一批。
    void EvaluateDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                            const std::unordered_set<ComputerNodeId>& currentBatchNodeIds);

    // 只在本批 dirty 节点内部做拓扑排序，保证“生产某个输入的节点”先于“消费它的节点”执行。
    void BatchNodeSort(std::queue<ComputerNodeId>& dirtyNodeQueue,
                       const std::unordered_set<ComputerNodeId>& dirtyNodeIds);
    // outputPropertyAddress 不能被简单丢弃：它可能还要触发本批之外的节点。
    // 但它在本批中已经消费过的节点，需要记录下来，避免下一批因为同一个 value 重复执行。
    bool SuppressCurrentBatchConsumers(const PropertyAddress& outputPropertyAddress,
                                       const std::unordered_set<ComputerNodeId>& currentBatchNodeIds);
    void ClearFlowTraceState();
    void RecordNodeTrigger(ComputerNodeId nodeId, const PropertyAddress& triggerPropertyAddress);
    void RecordChangedValue(const PropertyAddress& address);
    void RemoveDirtyValue(const PropertyAddress& address);

private:
    std::queue<PropertyAddress> dirtyQueue;
    std::unordered_set<PropertyAddress> m_DirtyValues;
    std::unordered_set<PropertyAddress> m_ChangedValues;
    std::unordered_map<ComputerNodeId, std::unordered_set<PropertyAddress>> m_NodeTriggerValues;
    // value -> 本批已经被满足的消费者节点。
    // 当这个 value 进入下一批 CollectDirtyNodes 时，这些消费者会被跳过，其余消费者照常触发。
    std::unordered_map<PropertyAddress, std::unordered_set<ComputerNodeId>> m_SuppressedConsumers;
    std::unordered_map<PropertyAddress, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<PropertyAddress>> nodeOutputs;
    std::unordered_map<PropertyAddress, ComputerNodeId> valueProducers;
};

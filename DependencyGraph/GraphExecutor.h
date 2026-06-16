#pragma once

#include "DependencyGraphIds.h"

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct DGContext;

struct GraphExecutor
{
    using FlowTraceCallback = std::function<void(const std::vector<std::string>&)>;

    void MarkDirty(ValueId id, const DGContext* context = nullptr);
    bool Evaluate(DGContext* context);
    void SetTraceEnabled(bool enabled);
    [[nodiscard]] bool IsTraceEnabled() const;
    void SetFlowTraceCallback(FlowTraceCallback callback);

    void AddDependentNode(ValueId valueId, ComputerNodeId nodeId);
    void AddNodeOutput(ComputerNodeId nodeId, ValueId valueId);

    [[nodiscard]] const std::vector<ComputerNodeId>* FindDependentNodes(ValueId valueId) const;
    [[nodiscard]] const std::vector<ValueId>* FindNodeOutputs(ComputerNodeId nodeId) const;

private:
    struct FlowEvent
    {
        ValueId trigger;
        ComputerNodeId node;
        std::vector<ValueId> outputs;
    };

    // 消费当前 dirty value 队列，收集本批需要执行的计算节点。
    // 注意：这里不会执行节点，只是把“哪些节点被脏值触发”转成一个 batch。
    void CollectDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                           std::unordered_set<ComputerNodeId>& dirtyNodeIds);
    // 按已经排好的 batch 顺序执行节点，并把输出 value 继续标脏，驱动下一批。
    void EvaluateDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                            const std::unordered_set<ComputerNodeId>& currentBatchNodeIds);

    // 只在本批 dirty 节点内部做拓扑排序，保证“生产某个输入的节点”先于“消费它的节点”执行。
    void BatchNodeSort(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                       const std::unordered_set<ComputerNodeId>& dirtyNodeIds);
    // outputValueId 不能被简单丢弃：它可能还要触发本批之外的节点。
    // 但它在本批中已经消费过的节点，需要记录下来，避免下一批因为同一个 value 重复执行。
    bool SuppressCurrentBatchConsumers(ValueId outputValueId,
                                       const std::unordered_set<ComputerNodeId>& currentBatchNodeIds);
    void ClearFlowTraceState();
    void RecordNodeTrigger(ComputerNodeId nodeId, ValueId triggerValueId);
    void RecordNodeOutputs(ComputerNodeId nodeId, const std::vector<ValueId>& outputs);
    void EmitFlowSummary(const DGContext* context) const;

private:
    std::queue<ValueId> dirtyQueue;
    std::unordered_set<ValueId> m_DirtyValues;
    bool m_TraceEnabled = false;
    FlowTraceCallback m_FlowTraceCallback;
    std::unordered_map<ComputerNodeId, std::unordered_set<ValueId>> m_NodeTriggerValues;
    std::vector<FlowEvent> m_FlowEvents;
    // value -> 本批已经被满足的消费者节点。
    // 当这个 value 进入下一批 CollectDirtyNodes 时，这些消费者会被跳过，其余消费者照常触发。
    std::unordered_map<ValueId, std::unordered_set<ComputerNodeId>> m_SuppressedConsumers;
    std::unordered_map<ValueId, std::vector<ComputerNodeId>> dependNodeLists;
    std::unordered_map<ComputerNodeId, std::vector<ValueId>> nodeOutputs;
};

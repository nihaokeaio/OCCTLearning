#include "GraphExecutor.h"

#include "ComputerNode.h"
#include "DGContext.h"

#include <algorithm>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <utility>

#ifdef DG_ENABLE_TRACE
#include <iostream>

namespace
{
    std::string ValueTraceLabel(const DGContext* context, const ValueId& id)
    {
        if (context != nullptr)
        {
            return context->ValueLabel(id);
        }
        return std::string("Value#") + std::to_string(id.m_Id);
    }

    void Trace(const std::string& message)
    {
        std::cout << "[DGTrace] " << message << std::endl;
    }

}
#endif

void GraphExecutor::MarkDirty(ValueId id, const DGContext* context)
{
    RecordChangedValue(id);
    if (m_DirtyValues.insert(id).second)
    {
        dirtyQueue.push(id);
#ifdef DG_ENABLE_TRACE
        if (m_TraceEnabled)
        {
            Trace("MarkDirty " + ValueTraceLabel(context, id));
        }
#endif
    }
#ifdef DG_ENABLE_TRACE
    else
    {
        if (m_TraceEnabled)
        {
            Trace("SkipDirtyAlreadyQueued " + ValueTraceLabel(context, id));
        }
    }
#endif
}

GraphExecutor::EvaluationResult GraphExecutor::Evaluate(DGContext* context)
{
    size_t batchIndex = 0;
    EvaluationResult result;
    ClearFlowTraceState();
    while (!dirtyQueue.empty())
    {
        ++batchIndex;
        std::queue<ComputerNodeId> dirtyNodeQueue;
        std::unordered_set<ComputerNodeId> dirtyNodeIds;
#ifdef DG_ENABLE_TRACE
        if (m_TraceEnabled)
        {
            Trace("BeginBatch #" + std::to_string(batchIndex));
        }
#endif
        CollectDirtyNodes(context, dirtyNodeQueue, dirtyNodeIds);
        BatchNodeSort(context, dirtyNodeQueue, dirtyNodeIds);
        if (!dirtyNodeIds.empty())
        {
            result.evaluated = true;
        }
        EvaluateDirtyNodes(context, dirtyNodeQueue, dirtyNodeIds);
#ifdef DG_ENABLE_TRACE
        if (m_TraceEnabled)
        {
            Trace("EndBatch #" + std::to_string(batchIndex));
        }
#endif
    }
    EmitFlowSummary(context);
    result.changedValues = m_ChangedValues;
    m_ChangedValues.clear();
    return result;
}

void GraphExecutor::SetTraceEnabled(bool enabled)
{
    m_TraceEnabled = enabled;
}

bool GraphExecutor::IsTraceEnabled() const
{
    return m_TraceEnabled;
}

void GraphExecutor::SetFlowTraceCallback(FlowTraceCallback callback)
{
    m_FlowTraceCallback = std::move(callback);
}

void GraphExecutor::AddDependentNode(ValueId valueId, ComputerNodeId nodeId)
{
    dependNodeLists[valueId].push_back(nodeId);
}

void GraphExecutor::AddNodeOutput(ComputerNodeId nodeId, ValueId valueId)
{
    nodeOutputs[nodeId].push_back(valueId);
    valueProducers.insert_or_assign(valueId, nodeId);
}

void GraphExecutor::RemoveNode(ComputerNodeId nodeId)
{
    for (auto iter = dependNodeLists.begin(); iter != dependNodeLists.end();)
    {
        auto& nodes = iter->second;
        std::erase(nodes, nodeId);
        if (nodes.empty())
        {
            iter = dependNodeLists.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    if (const auto outputsIter = nodeOutputs.find(nodeId); outputsIter != nodeOutputs.end())
    {
        for (const auto& outputValueId : outputsIter->second)
        {
            if (const auto producerIter = valueProducers.find(outputValueId);
                producerIter != valueProducers.end() && producerIter->second == nodeId)
            {
                valueProducers.erase(producerIter);
            }
        }
        nodeOutputs.erase(outputsIter);
    }

    m_NodeTriggerValues.erase(nodeId);
    for (auto iter = m_SuppressedConsumers.begin(); iter != m_SuppressedConsumers.end();)
    {
        iter->second.erase(nodeId);
        if (iter->second.empty())
        {
            iter = m_SuppressedConsumers.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    std::erase_if(m_FlowEvents, [nodeId](const FlowEvent& event)
    {
        return event.node == nodeId;
    });
}

void GraphExecutor::RemoveValue(ValueId valueId)
{
    dependNodeLists.erase(valueId);
    valueProducers.erase(valueId);
    m_DirtyValues.erase(valueId);
    m_ChangedValues.erase(valueId);
    m_SuppressedConsumers.erase(valueId);
    RemoveDirtyValue(valueId);

    for (auto& [nodeId, outputs] : nodeOutputs)
    {
        std::erase(outputs, valueId);
    }
    for (auto& [nodeId, triggerValues] : m_NodeTriggerValues)
    {
        triggerValues.erase(valueId);
    }
    std::erase_if(m_FlowEvents, [valueId](const FlowEvent& event)
    {
        return event.trigger == valueId ||
            std::find(event.outputs.begin(), event.outputs.end(), valueId) != event.outputs.end();
    });
}

void GraphExecutor::Clear()
{
    std::queue<ValueId> emptyDirtyQueue;
    dirtyQueue.swap(emptyDirtyQueue);
    m_DirtyValues.clear();
    m_ChangedValues.clear();
    m_NodeTriggerValues.clear();
    m_FlowEvents.clear();
    m_SuppressedConsumers.clear();
    dependNodeLists.clear();
    nodeOutputs.clear();
    valueProducers.clear();
}

const std::vector<ComputerNodeId>* GraphExecutor::FindDependentNodes(ValueId valueId) const
{
    const auto iter = dependNodeLists.find(valueId);
    if (iter == dependNodeLists.end())
    {
        return nullptr;
    }
    return &iter->second;
}

const std::vector<ValueId>* GraphExecutor::FindNodeOutputs(ComputerNodeId nodeId) const
{
    const auto iter = nodeOutputs.find(nodeId);
    if (iter == nodeOutputs.end())
    {
        return nullptr;
    }
    return &iter->second;
}

const ComputerNodeId* GraphExecutor::FindProducerNode(ValueId valueId) const
{
    const auto iter = valueProducers.find(valueId);
    if (iter == valueProducers.end())
    {
        return nullptr;
    }
    return &iter->second;
}

void GraphExecutor::CollectDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                                      std::unordered_set<ComputerNodeId>& dirtyNodeIds)
{
    while (!dirtyQueue.empty())
    {
        auto dirtyValueId = dirtyQueue.front();
        dirtyQueue.pop();
        m_DirtyValues.erase(dirtyValueId);
#ifdef DG_ENABLE_TRACE
        if (m_TraceEnabled)
        {
            Trace("PopDirty " + context->ValueLabel(dirtyValueId));
        }
#endif
        const auto& iter = dependNodeLists.find(dirtyValueId);
        if (iter == dependNodeLists.end())
        {
#ifdef DG_ENABLE_TRACE
            if (m_TraceEnabled)
            {
                Trace("NoDependents " + context->ValueLabel(dirtyValueId));
            }
#endif
            continue;
        }

        for (auto& computerNodeId : iter->second)
        {
            const auto suppressedIter = m_SuppressedConsumers.find(dirtyValueId);
            // 这个 value 是上一批节点的输出，所以它会进入下一批 dirty 队列。
            // 如果某些消费者已经在上一批里执行过，并且拓扑排序保证它们读到的是新值，
            // 这里就跳过这些消费者，只让尚未处理过的下游节点继续被触发。
            if (suppressedIter != m_SuppressedConsumers.end() &&
                suppressedIter->second.contains(computerNodeId))
            {
#ifdef DG_ENABLE_TRACE
                if (m_TraceEnabled)
                {
                    Trace("SkipNodeSatisfiedInPreviousBatch " + context->NodeLabel(computerNodeId) + " because " +
                        context->ValueLabel(dirtyValueId));
                }
#endif
                continue;
            }

            RecordNodeTrigger(computerNodeId, dirtyValueId);
            if (dirtyNodeIds.insert(computerNodeId).second)
            {
                dirtyNodeQueue.push(computerNodeId);
#ifdef DG_ENABLE_TRACE
                if (m_TraceEnabled)
                {
                    Trace("QueueNode " + context->NodeLabel(computerNodeId) + " because " +
                        context->ValueLabel(dirtyValueId));
                }
#endif
            }
#ifdef DG_ENABLE_TRACE
            else
            {
                if (m_TraceEnabled)
                {
                    Trace("SkipNodeAlreadyQueued " + context->NodeLabel(computerNodeId) + " because " +
                        context->ValueLabel(dirtyValueId));
                }
            }
#endif
        }
        m_SuppressedConsumers.erase(dirtyValueId);
    }
}

void GraphExecutor::EvaluateDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                                       const std::unordered_set<ComputerNodeId>& currentBatchNodeIds)
{
    while (!dirtyNodeQueue.empty())
    {
        auto computerNodeId = dirtyNodeQueue.front();
        dirtyNodeQueue.pop();

        const auto node = context->GetNode(computerNodeId);
#ifdef DG_ENABLE_TRACE
        if (m_TraceEnabled)
        {
            Trace("EvaluateNode " + context->NodeLabel(computerNodeId));
        }
#endif
        node->Evaluator(*context);

        auto outputIter = nodeOutputs.find(computerNodeId);
        if (outputIter != nodeOutputs.end())
        {
            RecordNodeOutputs(computerNodeId, outputIter->second);
            for (const auto& outputValueId : outputIter->second)
            {
                RecordChangedValue(outputValueId);
                if (!dependNodeLists.contains(outputValueId))
                {
#ifdef DG_ENABLE_TRACE
                    if (m_TraceEnabled)
                    {
                        Trace("OutputHasNoDependents " + context->NodeLabel(computerNodeId) + " -> " +
                            context->ValueLabel(outputValueId));
                    }
#endif
                    continue;
                }

                // outputValueId 有两类消费者：
                // 1. 已经在 currentBatchNodeIds 中的消费者：本批拓扑排序会保证它们已经读到最新输出。
                // 2. 不在 currentBatchNodeIds 中的消费者：需要通过 MarkDirty 留给下一批继续触发。
                // 因此这里不能简单丢弃 outputValueId，只能把第 1 类消费者记为“下批跳过”。
                const bool hasConsumersOutsideCurrentBatch =
                    SuppressCurrentBatchConsumers(outputValueId, currentBatchNodeIds);
                if (!hasConsumersOutsideCurrentBatch)
                {
#ifdef DG_ENABLE_TRACE
                    if (m_TraceEnabled)
                    {
                        Trace("OutputConsumersAlreadyInBatch " + context->NodeLabel(computerNodeId) + " -> " +
                            context->ValueLabel(outputValueId));
                    }
#endif
                    continue;
                }

#ifdef DG_ENABLE_TRACE
                if (m_TraceEnabled)
                {
                    Trace("PropagateOutput " + context->NodeLabel(computerNodeId) + " -> " +
                        context->ValueLabel(outputValueId));
                }
#endif
                MarkDirty(outputValueId, context);
            }
        }
    }
}

void GraphExecutor::BatchNodeSort(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                                  const std::unordered_set<ComputerNodeId>& dirtyNodeIds)
{
    std::unordered_map<ComputerNodeId, std::unordered_set<ComputerNodeId>> downstreamNodes;
    std::unordered_map<ComputerNodeId, size_t> inDegrees;
    for (const auto& nodeId : dirtyNodeIds)
    {
        inDegrees[nodeId] = 0;
    }

    for (const auto& nodeId : dirtyNodeIds)
    {
        auto outputValueIter = nodeOutputs.find(nodeId);
        if (outputValueIter != nodeOutputs.end())
        {
            for (const auto& outputValueId : outputValueIter->second)
            {
                auto outputNodeIdIter = dependNodeLists.find(outputValueId);
                if (outputNodeIdIter != dependNodeLists.end())
                {
                    for (const auto& outputNodeId : outputNodeIdIter->second)
                    {
                        if (dirtyNodeIds.contains(outputNodeId) && outputNodeId != nodeId)
                        {
                            if (downstreamNodes[nodeId].insert(outputNodeId).second)
                            {
                                ++inDegrees[outputNodeId];
                            }
                        }
                    }
                }
            }
        }
    }

    std::queue<ComputerNodeId> readyNodeQueue;
    for (const auto& [nodeId, inDegree] : inDegrees)
    {
        if (inDegree == 0)
        {
            readyNodeQueue.push(nodeId);
        }
    }

    std::queue<ComputerNodeId> sortedNodeQueue;
    size_t sortedCount = 0;
    while (!readyNodeQueue.empty())
    {
        const auto nodeId = readyNodeQueue.front();
        readyNodeQueue.pop();
        sortedNodeQueue.push(nodeId);
        ++sortedCount;

        const auto downstreamIter = downstreamNodes.find(nodeId);
        if (downstreamIter == downstreamNodes.end())
        {
            continue;
        }

        for (const auto& downstreamNodeId : downstreamIter->second)
        {
            auto& inDegree = inDegrees[downstreamNodeId];
            --inDegree;
            if (inDegree == 0)
            {
                readyNodeQueue.push(downstreamNodeId);
            }
        }
    }

    if (sortedCount != dirtyNodeIds.size())
    {
        throw std::runtime_error("Cycle detected inside dirty node batch");
    }

    dirtyNodeQueue.swap(sortedNodeQueue);
}

bool GraphExecutor::SuppressCurrentBatchConsumers(
    ValueId outputValueId,
    const std::unordered_set<ComputerNodeId>& currentBatchNodeIds)
{
    const auto dependentIter = dependNodeLists.find(outputValueId);
    if (dependentIter == dependNodeLists.end())
    {
        return false;
    }

    bool hasConsumersOutsideCurrentBatch = false;
    for (const auto& dependentNodeId : dependentIter->second)
    {
        if (currentBatchNodeIds.contains(dependentNodeId))
        {
            // 本批内消费者已经由 BatchNodeSort 安排在正确位置，下一批不应再因为同一个输出重复执行。
            m_SuppressedConsumers[outputValueId].insert(dependentNodeId);
        }
        else
        {
            hasConsumersOutsideCurrentBatch = true;
        }
    }
    return hasConsumersOutsideCurrentBatch;
}

void GraphExecutor::ClearFlowTraceState()
{
    m_NodeTriggerValues.clear();
    m_FlowEvents.clear();
}

void GraphExecutor::RecordNodeTrigger(ComputerNodeId nodeId, ValueId triggerValueId)
{
    m_NodeTriggerValues[nodeId].insert(triggerValueId);
}

void GraphExecutor::RecordNodeOutputs(ComputerNodeId nodeId, const std::vector<ValueId>& outputs)
{
    const auto triggerIter = m_NodeTriggerValues.find(nodeId);
    if (triggerIter == m_NodeTriggerValues.end())
    {
        return;
    }

    for (const auto& triggerValueId : triggerIter->second)
    {
        m_FlowEvents.push_back({triggerValueId, nodeId, outputs});
    }
}

void GraphExecutor::EmitFlowSummary(const DGContext* context) const
{
    if (!m_FlowTraceCallback || m_FlowEvents.empty() || context == nullptr)
    {
        return;
    }

    std::vector<std::string> lines;
    for (const auto& event : m_FlowEvents)
    {
        std::ostringstream out;
        out << context->ValueLabel(event.trigger)
            << " -> " << context->NodeLabel(event.node);

        if (!event.outputs.empty())
        {
            out << " -> ";
            for (size_t i = 0; i < event.outputs.size(); ++i)
            {
                if (i != 0)
                {
                    out << ", ";
                }
                out << context->ValueLabel(event.outputs[i]);
            }
        }
        lines.push_back(out.str());
    }

    if (!lines.empty())
    {
        m_FlowTraceCallback(lines);
    }
}

void GraphExecutor::RecordChangedValue(ValueId valueId)
{
    m_ChangedValues.insert(valueId);
}

void GraphExecutor::RemoveDirtyValue(ValueId valueId)
{
    std::queue<ValueId> retainedValues;
    while (!dirtyQueue.empty())
    {
        const auto currentValueId = dirtyQueue.front();
        dirtyQueue.pop();
        if (!(currentValueId == valueId))
        {
            retainedValues.push(currentValueId);
        }
    }
    dirtyQueue.swap(retainedValues);
}

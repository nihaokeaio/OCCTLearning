#include "GraphExecutor.h"

#include "ComputerNode.h"
#include "DGContext.h"

#include <stdexcept>

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
    if (m_DirtyValues.insert(id).second)
    {
        dirtyQueue.push(id);
#ifdef DG_ENABLE_TRACE
        Trace("MarkDirty " + ValueTraceLabel(context, id));
#endif
    }
#ifdef DG_ENABLE_TRACE
    else
    {
        Trace("SkipDirtyAlreadyQueued " + ValueTraceLabel(context, id));
    }
#endif
}

bool GraphExecutor::Evaluate(DGContext* context)
{
    size_t batchIndex = 0;
    while (!dirtyQueue.empty())
    {
        ++batchIndex;
        std::queue<ComputerNodeId> dirtyNodeQueue;
        std::unordered_set<ComputerNodeId> dirtyNodeIds;
#ifdef DG_ENABLE_TRACE
        Trace("BeginBatch #" + std::to_string(batchIndex));
#endif
        CollectDirtyNodes(context, dirtyNodeQueue, dirtyNodeIds);
        BatchNodeSort(context, dirtyNodeQueue, dirtyNodeIds);
        EvaluateDirtyNodes(context, dirtyNodeQueue, dirtyNodeIds);
#ifdef DG_ENABLE_TRACE
        Trace("EndBatch #" + std::to_string(batchIndex));
#endif
    }
    return false;
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
        Trace("PopDirty " + context->ValueLabel(dirtyValueId));
#endif
        const auto& iter = dependNodeLists.find(dirtyValueId);
        if (iter == dependNodeLists.end())
        {
#ifdef DG_ENABLE_TRACE
            Trace("NoDependents " + context->ValueLabel(dirtyValueId));
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
                Trace("SkipNodeSatisfiedInPreviousBatch " + context->NodeLabel(computerNodeId) + " because " +
                    context->ValueLabel(dirtyValueId));
#endif
                continue;
            }

            if (dirtyNodeIds.insert(computerNodeId).second)
            {
                dirtyNodeQueue.push(computerNodeId);
#ifdef DG_ENABLE_TRACE
                Trace("QueueNode " + context->NodeLabel(computerNodeId) + " because " +
                    context->ValueLabel(dirtyValueId));
#endif
            }
#ifdef DG_ENABLE_TRACE
            else
            {
                Trace("SkipNodeAlreadyQueued " + context->NodeLabel(computerNodeId) + " because " +
                    context->ValueLabel(dirtyValueId));
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

        const auto node = context->m_Nodes[computerNodeId].get();
#ifdef DG_ENABLE_TRACE
        Trace("EvaluateNode " + context->NodeLabel(computerNodeId));
#endif
        node->Evaluator(*context);

        auto outputIter = nodeOutputs.find(computerNodeId);
        if (outputIter != nodeOutputs.end())
        {
            for (const auto& outputValueId : outputIter->second)
            {
                if (!dependNodeLists.contains(outputValueId))
                {
#ifdef DG_ENABLE_TRACE
                    Trace("OutputHasNoDependents " + context->NodeLabel(computerNodeId) + " -> " +
                        context->ValueLabel(outputValueId));
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
                    Trace("OutputConsumersAlreadyInBatch " + context->NodeLabel(computerNodeId) + " -> " +
                        context->ValueLabel(outputValueId));
#endif
                    continue;
                }

#ifdef DG_ENABLE_TRACE
                Trace("PropagateOutput " + context->NodeLabel(computerNodeId) + " -> " +
                    context->ValueLabel(outputValueId));
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

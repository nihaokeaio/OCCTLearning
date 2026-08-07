#include "GraphExecutor.h"

#include "ComputerNode.h"
#include "DGContext.h"
#include <queue>
#include <stdexcept>
#include <utility>


void GraphExecutor::MarkDirty(const PropertyAddress& address)
{
    RecordChangedValue(address);
    if (m_DirtyValues.insert(address).second)
    {
        dirtyQueue.push(address);
    }
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
        CollectDirtyNodes(dirtyNodeQueue, dirtyNodeIds);
        BatchNodeSort(dirtyNodeQueue, dirtyNodeIds);
        if (!dirtyNodeIds.empty())
        {
            result.evaluated = true;
        }
        EvaluateDirtyNodes(context, dirtyNodeQueue, dirtyNodeIds);
    }
    result.changedValues = m_ChangedValues;
    m_ChangedValues.clear();
    return result;
}

void GraphExecutor::AddDependentNode(const PropertyAddress& address, const ComputerNodeId nodeId)
{
    dependNodeLists[address].push_back(nodeId);
}

void GraphExecutor::AddNodeOutput(ComputerNodeId nodeId, const PropertyAddress& address)
{
    nodeOutputs[nodeId].push_back(address);
    valueProducers.insert_or_assign(address, nodeId);
}

void GraphExecutor::RemoveComputerNode(ComputerNodeId nodeId)
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
        for (const auto& outputPropertyAddress : outputsIter->second)
        {
            if (const auto producerIter = valueProducers.find(outputPropertyAddress);
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
}

void GraphExecutor::RemoveValueAddress(const PropertyAddress& address)
{
    dependNodeLists.erase(address);
    valueProducers.erase(address);
    m_DirtyValues.erase(address);
    m_ChangedValues.erase(address);
    m_SuppressedConsumers.erase(address);
    RemoveDirtyValue(address);

    for (auto& outputs : nodeOutputs | std::views::values)
    {
        std::erase(outputs, address);
    }
    for (auto& triggerValues : m_NodeTriggerValues | std::views::values)
    {
        triggerValues.erase(address);
    }
}

void GraphExecutor::Clear()
{
    std::queue<PropertyAddress> emptyDirtyQueue;
    dirtyQueue.swap(emptyDirtyQueue);
    m_DirtyValues.clear();
    m_ChangedValues.clear();
    m_NodeTriggerValues.clear();
    m_SuppressedConsumers.clear();
    dependNodeLists.clear();
    nodeOutputs.clear();
    valueProducers.clear();
}

std::span<const ComputerNodeId> GraphExecutor::FindDependentNodes(const PropertyAddress& address) const
{
    const auto iter = dependNodeLists.find(address);
    if (iter == dependNodeLists.end())
    {
        return {};
    }
    return std::span(iter->second);
}

std::span<const PropertyAddress> GraphExecutor::FindNodeOutputs(const ComputerNodeId nodeId) const
{
    const auto iter = nodeOutputs.find(nodeId);
    if (iter == nodeOutputs.end())
    {
        return {};
    }
    return std::span(iter->second);
}

const ComputerNodeId* GraphExecutor::FindProducerNode(const PropertyAddress& address) const
{
    const auto iter = valueProducers.find(address);
    if (iter == valueProducers.end())
    {
        return nullptr;
    }
    return &iter->second;
}

void GraphExecutor::CollectDirtyNodes(std::queue<ComputerNodeId>& dirtyNodeQueue,
                                      std::unordered_set<ComputerNodeId>& dirtyNodeIds)
{
    while (!dirtyQueue.empty())
    {
        auto dirtyPropertyAddress = dirtyQueue.front();
        dirtyQueue.pop();
        m_DirtyValues.erase(dirtyPropertyAddress);
        const auto& iter = dependNodeLists.find(dirtyPropertyAddress);
        if (iter == dependNodeLists.end())
        {
            continue;
        }

        for (auto& computerNodeId : iter->second)
        {
            const auto suppressedIter = m_SuppressedConsumers.find(dirtyPropertyAddress);
            // 这个 value 是上一批节点的输出，所以它会进入下一批 dirty 队列。
            // 如果某些消费者已经在上一批里执行过，并且拓扑排序保证它们读到的是新值，
            // 这里就跳过这些消费者，只让尚未处理过的下游节点继续被触发。
            if (suppressedIter != m_SuppressedConsumers.end() &&
                suppressedIter->second.contains(computerNodeId))
            {
                continue;
            }

            RecordNodeTrigger(computerNodeId, dirtyPropertyAddress);
            if (dirtyNodeIds.insert(computerNodeId).second)
            {
                dirtyNodeQueue.push(computerNodeId);
            }
        }
        m_SuppressedConsumers.erase(dirtyPropertyAddress);
    }
}

void GraphExecutor::EvaluateDirtyNodes(DGContext* context, std::queue<ComputerNodeId>& dirtyNodeQueue,
                                       const std::unordered_set<ComputerNodeId>& currentBatchNodeIds)
{
    while (!dirtyNodeQueue.empty())
    {
        auto computerNodeId = dirtyNodeQueue.front();
        dirtyNodeQueue.pop();

        const auto node = context->GetComputerNode(computerNodeId);
        node->Evaluator(*context);

        auto outputIter = nodeOutputs.find(computerNodeId);
        if (outputIter != nodeOutputs.end())
        {
            for (const auto& outputPropertyAddress : outputIter->second)
            {
                RecordChangedValue(outputPropertyAddress);
                if (!dependNodeLists.contains(outputPropertyAddress))
                {
                    continue;
                }
                // outputPropertyAddress 有两类消费者：
                // 1. 已经在 currentBatchNodeIds 中的消费者：本批拓扑排序会保证它们已经读到最新输出。
                // 2. 不在 currentBatchNodeIds 中的消费者：需要通过 MarkDirty 留给下一批继续触发。
                // 因此这里不能简单丢弃 outputPropertyAddress，只能把第 1 类消费者记为“下批跳过”。
                const bool hasConsumersOutsideCurrentBatch =
                    SuppressCurrentBatchConsumers(outputPropertyAddress, currentBatchNodeIds);
                if (!hasConsumersOutsideCurrentBatch)
                {
                    continue;
                }
                MarkDirty(outputPropertyAddress);
            }
        }
    }
}

void GraphExecutor::BatchNodeSort(std::queue<ComputerNodeId>& dirtyNodeQueue,
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
            for (const auto& outputPropertyAddress : outputValueIter->second)
            {
                auto outputNodeIdIter = dependNodeLists.find(outputPropertyAddress);
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
    const PropertyAddress& outputPropertyAddress,
    const std::unordered_set<ComputerNodeId>& currentBatchNodeIds)
{
    const auto dependentIter = dependNodeLists.find(outputPropertyAddress);
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
            m_SuppressedConsumers[outputPropertyAddress].insert(dependentNodeId);
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
}

void GraphExecutor::RecordNodeTrigger(ComputerNodeId nodeId, const PropertyAddress& triggerPropertyAddress)
{
    m_NodeTriggerValues[nodeId].insert(triggerPropertyAddress);
}

void GraphExecutor::RecordChangedValue(const PropertyAddress& address)
{
    m_ChangedValues.insert(address);
}

void GraphExecutor::RemoveDirtyValue(const PropertyAddress& address)
{
    std::queue<PropertyAddress> retainedValues;
    while (!dirtyQueue.empty())
    {
        const auto currentPropertyAddress = dirtyQueue.front();
        dirtyQueue.pop();
        if (currentPropertyAddress != address)
        {
            retainedValues.push(currentPropertyAddress);
        }
    }
    dirtyQueue.swap(retainedValues);
}

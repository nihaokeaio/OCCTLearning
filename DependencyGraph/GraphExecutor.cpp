#include "GraphExecutor.h"

#include "ComputerNode.h"
#include "DGContext.h"

void GraphExecutor::MarkDirty(ValueId id)
{
    dirtyQueue.push(id);
}

bool GraphExecutor::Evaluate(DGContext* context)
{
    while (!dirtyQueue.empty())
    {
        auto dirtyValueId = dirtyQueue.front();
        dirtyQueue.pop();
        const auto& iter = dependNodeLists.find(dirtyValueId);
        if (iter == dependNodeLists.end())
        {
            continue;
        }
        for (auto& computerNodeId : iter->second)
        {
            const auto node = context->m_Nodes[computerNodeId].get();
            node->Evaluator(*context);

            auto outputIter = nodeOutputs.find(computerNodeId);
            if (outputIter != nodeOutputs.end())
            {
                for (const auto& outputValueId : outputIter->second)
                {
                    MarkDirty(outputValueId);
                }
            }
        }
    }
    return false;
}

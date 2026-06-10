#include "GraphExecutor.h"

#include "ComputerNode.h"
#include "DGContext.h"

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
            const auto node = context->m_Nodes[computerNodeId].get();
#ifdef DG_ENABLE_TRACE
            Trace("EvaluateNode " + context->NodeLabel(computerNodeId) + " because " +
                context->ValueLabel(dirtyValueId));
#endif
            node->Evaluator(*context);

            auto outputIter = nodeOutputs.find(computerNodeId);
            if (outputIter != nodeOutputs.end())
            {
                for (const auto& outputValueId : outputIter->second)
                {
#ifdef DG_ENABLE_TRACE
                    Trace("PropagateOutput " + context->NodeLabel(computerNodeId) + " -> " +
                        context->ValueLabel(outputValueId));
#endif
                    MarkDirty(outputValueId, context);
                }
            }
        }
    }
    return false;
}

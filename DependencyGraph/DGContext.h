#pragma once

#include "ComputerNode.h"
#include "GraphExecutor.h"
#include "Render.h"
#include "ValueHandle.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct DGContext
{
    DGContext();

    ComputerNode* GetComputerNode(const ComputerNodeId& id);

    ValueHandle& GetValueHandle(const ValueId& id);
    const ValueHandle& GetValueHandle(const ValueId& id) const;

    void AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<ValueId>& inputs,
                         const std::vector<ValueId>& outputs);

    ComputerNode* AddComputeNode(const std::vector<ValueId>& inputs,
                                 const std::vector<ValueId>& outputs,
                                 ComputerNode::ComputeFunc computeFunc);

    ValueId AddValueHandle(std::unique_ptr<ValueHandle>&& valueHandle);

    template <class T>
    ValueId CreateValue(const std::string& propertyName, const T& initialValue);

    template <class T>
    void SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value);

    void Evaluator();

    std::unordered_map<ValueId, std::unique_ptr<ValueHandle>> m_Values;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unique_ptr<Render> render;
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
    m_GraphExecutor->MarkDirty(valueId);
}

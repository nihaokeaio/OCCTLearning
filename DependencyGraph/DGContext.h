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

    void AddComputerNode(std::unique_ptr<ComputerNode>&& node, const std::vector<std::shared_ptr<ValueHandle>>& inputs,
                         const std::vector<std::shared_ptr<ValueHandle>>& outputs);

    void AddValueHandle(const std::shared_ptr<ValueHandle>& valueHandle);

    template <class T>
    std::shared_ptr<ValueHandle> CreateValue(const std::string& propertyName, const T& initialValue);

    template <class T>
    void SetValueProperty(const std::shared_ptr<ValueHandle>& valueHandle, const std::string& propertyName,
                          const T& value);

    template <class T>
    void SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value);

    void Evaluator();

    std::unordered_map<ValueId, std::shared_ptr<ValueHandle>> m_Values;
    std::unordered_map<ComputerNodeId, std::unique_ptr<ComputerNode>> m_Nodes;
    std::unique_ptr<Render> render;
    std::unique_ptr<GraphExecutor> m_GraphExecutor;
};

template <class T>
std::shared_ptr<ValueHandle> DGContext::CreateValue(const std::string& propertyName, const T& initialValue)
{
    auto valueHandle = std::make_shared<ValueHandle>();
    valueHandle->AddProperty(propertyName, initialValue);
    AddValueHandle(valueHandle);
    return valueHandle;
}

template <class T>
void DGContext::SetValueProperty(const std::shared_ptr<ValueHandle>& valueHandle, const std::string& propertyName,
                                 const T& value)
{
    valueHandle->SetProperty(propertyName, value);
    m_GraphExecutor->MarkDirty(valueHandle->m_Id);
}

template <class T>
void DGContext::SetValueProperty(const ValueId& valueId, const std::string& propertyName, const T& value)
{
    const auto iter = m_Values.find(valueId);
    if (iter == m_Values.end())
    {
        throw std::runtime_error("ValueHandle does not exist");
    }
    SetValueProperty(iter->second, propertyName, value);
}

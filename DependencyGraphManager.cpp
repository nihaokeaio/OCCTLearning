//
// Created by ZQD on 26-5-22.
//

#include "DependencyGraphManager.h"

PropertyValue::PropertyValue(int value)
{
    m_Value = value;
}

PropertyValue::PropertyValue(double value)
{
    m_Value = value;
}

PropertyValue::PropertyValue(bool value)
{
    m_Value = value;
}

PropertyValue::PropertyValue(const std::string& value)
{
    m_Value = value;
}

PropertyValue::PropertyValue(const gp_Pnt& value)
{
    m_Value = value;
}

void ValueHandle::SetProperty(const std::string& name, const PropertyValue& value)
{
    const auto it = properties.find(name);
    if (it == properties.end())
    {
        throw std::runtime_error("Property does not exist: " + name);
    }

    const PropertyValue oldValue = it->second;
    it->second = value;
    NotifyPropertyChanged(name, it->second, oldValue);
}

void ValueHandle::NotifyPropertyChanged(const std::string& name, const PropertyValue& newVal,
                                        const PropertyValue& oldVal)
{
    if (m_OnSetFun)
    {
        m_OnSetFun(*this, name, newVal, oldVal);
    }
}

DGContext::DGContext()
{
    m_GraphExecutor = std::make_unique<GraphExecutor>();
}

void DGContext::AddComputerNode(std::unique_ptr<ComputerNode>&& node,
                                const std::vector<std::shared_ptr<ValueHandle>>& inputs,
                                const std::vector<std::shared_ptr<ValueHandle>>& outputs)
{
    for (const auto& input : inputs)
    {
        m_GraphExecutor->dependNodeLists[input->m_Id].push_back(node->m_Id);
    }

    for (const auto& output : outputs)
    {
        m_GraphExecutor->nodeOutputs[node->m_Id].push_back(output->m_Id);
    }
    m_Nodes.insert({node->m_Id, std::move(node)});
}

void DGContext::AddValueHandle(const std::shared_ptr<ValueHandle>& valueHandle)
{
    m_Values.insert_or_assign(valueHandle->m_Id, valueHandle);
}

void DGContext::Evaluator()
{
    m_GraphExecutor->Evaluate(this);
}

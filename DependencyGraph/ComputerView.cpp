//
// Created by ZQD on 26-6-8.
//

#include "ComputerView.h"
#include "DGContext.h"
#include <stdexcept>
#include "Data/Document.h"
#include "Data/Element.h"

ComputerView::ComputerView(Document* document, std::span<PropertyAddress> inputs, std::span<PropertyAddress> outputs):
    m_Document(document),
    m_Inputs(std::move(inputs)),
    m_Outputs(std::move(outputs))
{
}

std::optional<PropertyValue> ComputerView::In(size_t index) const
{
    const auto& valueAddress = InId(index);
    return GetValue(valueAddress);
}

std::optional<PropertyValue> ComputerView::Out(size_t index) const
{
    const auto& valueAddress = OutId(index);
    return GetValue(valueAddress);
}

std::optional<PropertyValue> ComputerView::GetValue(const PropertyAddress& address) const
{
    const auto elementId = address.elementId;
    const auto& key = address.propertyName;
    if (const auto element = m_Document->FindElement(elementId))
    {
        return element->GetProperty(key);
    }
    return std::nullopt;
}

bool ComputerView::SetValue(const PropertyAddress& address, const PropertyValue& value) const
{
    const auto elementId = address.elementId;
    const auto& key = address.propertyName;
    if (const auto element = m_Document->FindElement(elementId))
    {
        element->SetProperty(key, value);
        return true;
    }
    return false;
}


PropertyAddress ComputerView::InId(size_t index) const
{
    if (index >= m_Inputs.size())
    {
        throw std::out_of_range("ComputerView input index out of range");
    }
    return m_Inputs[index];
}

PropertyAddress ComputerView::OutId(size_t index) const
{
    if (index >= m_Outputs.size())
    {
        throw std::out_of_range("ComputerView output index out of range");
    }
    return m_Outputs[index];
}

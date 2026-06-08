//
// Created by ZQD on 26-6-8.
//

#include "ComputerView.h"

#include "DGContext.h"
#include "ValueHandle.h"

#include <stdexcept>

ComputerView::ComputerView(DGContext& context, const std::vector<ValueId>& inputs,
                           const std::vector<ValueId>& outputs):
    m_Context(&context),
    m_Inputs(&inputs),
    m_Outputs(&outputs)
{
}

ValueHandle& ComputerView::In(size_t index) const
{
    return m_Context->GetValueHandle(InId(index));
}

ValueHandle& ComputerView::Out(size_t index) const
{
    return m_Context->GetValueHandle(OutId(index));
}

ValueId ComputerView::InId(size_t index) const
{
    if (index >= m_Inputs->size())
    {
        throw std::out_of_range("ComputerView input index out of range");
    }
    return (*m_Inputs)[index];
}

ValueId ComputerView::OutId(size_t index) const
{
    if (index >= m_Outputs->size())
    {
        throw std::out_of_range("ComputerView output index out of range");
    }
    return (*m_Outputs)[index];
}

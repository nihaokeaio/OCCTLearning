//
// Created by ZQD on 26-8-7.
//

#include "ElementId.h"

const ElementId ElementId::InvalidId(0);

ElementId::ElementId(const uint64_t id) : m_Value(id)
{
}

ElementId::ElementId(const std::string& value)
{
    if (value.empty())
    {
        m_Value = 0;
    }
    else
    {
        m_Value = std::stoull(value);
    }
}

ElementId::value_type ElementId::GetValue() const
{
    return m_Value;
}

bool ElementId::IsValid() const
{
    return m_Value > 0;
}


bool ElementId::operator==(const ElementId& rhs) const
{
    return m_Value == rhs.m_Value;
}

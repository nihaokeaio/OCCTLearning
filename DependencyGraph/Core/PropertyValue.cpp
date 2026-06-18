#include "PropertyValue.h"

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

const PropertyValue::Storage& PropertyValue::Get() const
{
    return m_Value;
}

void PropertyValue::Set(const Storage& val)
{
    m_Value = val;
}

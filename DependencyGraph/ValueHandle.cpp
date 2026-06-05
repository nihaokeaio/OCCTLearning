#include "ValueHandle.h"

#include <utility>

void ValueHandle::AddProperty(const std::string& name, const PropertyValue& initialValue)
{
    properties.insert_or_assign(name, initialValue);
}

bool ValueHandle::HasProperty(const std::string& name) const
{
    return properties.find(name) != properties.end();
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

void ValueHandle::SetOnSetProperty(PropertyChangedCallback fun)
{
    m_OnSetFun = std::move(fun);
}

void ValueHandle::NotifyPropertyChanged(const std::string& name, const PropertyValue& newVal,
                                        const PropertyValue& oldVal)
{
    if (m_OnSetFun)
    {
        m_OnSetFun(*this, name, newVal, oldVal);
    }
}

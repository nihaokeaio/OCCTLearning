#pragma once

#include "DependencyGraphIds.h"
#include "PropertyValue.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>

enum class ValueRole
{
    UserInput,
    Derived
};

[[nodiscard]] const char* ToString(ValueRole role);

struct ValueHandle
{
    using PropertyChangedCallback = std::function<void(ValueHandle& valueHandle,
                                                       const std::string& name,
                                                       const PropertyValue& newVal,
                                                       const PropertyValue& oldVal)>;

    virtual ~ValueHandle() = default;

    template <class T>
    void AddProperty(const std::string& name, const T& initialValue)
    {
        properties.insert_or_assign(name, PropertyValue(initialValue));
    }

    void AddProperty(const std::string& name, const PropertyValue& initialValue);

    template <class T>
    [[nodiscard]] T GetProperty(const std::string& name) const
    {
        const auto it = properties.find(name);
        if (it == properties.end())
        {
            throw std::runtime_error("Property does not exist: " + name);
        }
        return it->second.As<T>();
    }

    [[nodiscard]] bool HasProperty(const std::string& name) const;

    template <class T>
    void SetProperty(const std::string& name, const T& value)
    {
        const auto it = properties.find(name);
        if (it == properties.end())
        {
            throw std::runtime_error("Property does not exist: " + name);
        }

        const PropertyValue oldValue = it->second;
        it->second.Set(value);
        NotifyPropertyChanged(name, it->second, oldValue);
    }

    void SetProperty(const std::string& name, const PropertyValue& value);
    void SetOnSetProperty(PropertyChangedCallback fun);

    ValueId m_Id;
    ValueRole m_Role = ValueRole::UserInput;
    std::unordered_map<std::string, PropertyValue> properties;
    PropertyChangedCallback m_OnSetFun;

private:
    void NotifyPropertyChanged(const std::string& name, const PropertyValue& newVal, const PropertyValue& oldVal);
};

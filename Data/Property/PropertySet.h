//
// Created by ZQD on 2026/6/3.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "MetaObjectManager.h"


using PropertyValue = MiniMetaObject::MetaValue;

class PropertySet
{
public:
    PropertySet();

    ~PropertySet();

    void Set(const std::string& key, const PropertyValue& value);

    template <typename T>
    void SetT(const std::string& key, const T& value);


    template <typename T>
    bool Get(const std::string& key, T& value) const;

    [[nodiscard]] std::optional<PropertyValue> Get(const std::string& key) const;

    void Remove(const std::string& key);

    void Clear();

    [[nodiscard]] bool Exists(const std::string& key) const;

    template <typename Owner, typename T>
    MiniMetaObject::MetaProperty static MakeMetaProperty(const std::string &name, const std::string &key) {
        using namespace MiniMetaObject;
        static_assert(std::derived_from<Owner, Object>);

        auto accessorGetter = [key](const Object* object)-> PropertyValue
        {
            const auto* owner = dynamic_cast<const Owner*>(object);
            if (!owner)
            {
                throw std::runtime_error("Invalid property owner");
            }
            std::optional<PropertyValue> value;
            if (value = owner->GetProperty(key); !value.has_value()) {
                throw std::runtime_error("Property does not exist: " + key);
            }
            if (!value.value().GetIf<T>()) {
                throw std::runtime_error("Property Type Invalid: ");
            }
            return value.value();
        };
        auto accessorSetter = [key](Object* object, const MetaValue& value)-> bool
        {
            auto* owner = dynamic_cast<Owner*>(object);
            if (!owner)
            {
                return false;
            }
            const T* typedValue = value.GetIf<T>();
            if (!typedValue)
            {
                return false;
            }
            owner->SetPropertyDirectly(key, *typedValue);
            return true;
        };
        return MetaProperty(name, GetMetaType<T>(), accessorGetter, accessorSetter);
    }

private:
    std::unordered_map<std::string, PropertyValue> m_Properties;
};

template <typename T>
void PropertySet::SetT(const std::string& key, const T& value)
{
    Set(key, PropertyValue(value));
}

template <typename T>
bool PropertySet::Get(const std::string& key, T& value) const
{
    if (const auto it = m_Properties.find(key); it != m_Properties.end())
    {
        value = it->second;
    }
    return false;
}



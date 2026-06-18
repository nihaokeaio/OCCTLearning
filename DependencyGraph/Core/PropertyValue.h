#pragma once

#include <gp_Pnt.hxx>

#include <string>
#include <variant>

struct PropertyValue
{
    using Storage = std::variant<std::monostate, bool, int, double, gp_Pnt, std::string>;

    PropertyValue() = default;
    PropertyValue(int value);
    PropertyValue(double value);
    PropertyValue(bool value);
    PropertyValue(const std::string& value);
    PropertyValue(const gp_Pnt& value);
    ~PropertyValue() = default;

    [[nodiscard]] const Storage& Get() const;

    template <class T>
    [[nodiscard]] const T& As() const
    {
        return std::get<T>(m_Value);
    }

    template <class T>
    [[nodiscard]] bool Is() const
    {
        return std::holds_alternative<T>(m_Value);
    }

    void Set(const Storage& val);

    template <class T>
    void Set(const T& val)
    {
        m_Value = val;
    }

    Storage m_Value;
};

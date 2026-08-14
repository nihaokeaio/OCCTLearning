//
// Created by ZQD on 26-6-8.
//
#pragma once

#include "Data/Property/PropertyAddress.h"
#include "Data/Property/PropertySet.h"

class Document;
struct DGContext;
struct ValueHandle;

struct ComputerView
{
    ComputerView(DGContext* context, std::span<PropertyAddress> inputs, std::span<PropertyAddress> outputs);

    template <typename T>
    T Input(size_t index) const;

    template <typename T>
    T Output(size_t index) const;

    template <typename T>
        requires std::is_constructible_v<PropertyValue, T>
    void SetOutput(size_t index, const T& value) const;

private:
    [[nodiscard]] PropertyAddress InId(size_t index) const;
    [[nodiscard]] PropertyAddress OutId(size_t index) const;

    [[nodiscard]] std::optional<PropertyValue> In(size_t index) const;
    [[nodiscard]] std::optional<PropertyValue> Out(size_t index) const;

    [[nodiscard]] std::optional<PropertyValue> GetValue(const PropertyAddress& address) const;
    [[nodiscard]] bool SetValue(const PropertyAddress& address, const PropertyValue& value) const;

private:
    DGContext* m_DGContext = nullptr;
    std::span<PropertyAddress> m_Inputs;
    std::span<PropertyAddress> m_Outputs;
};

template <typename T>
T ComputerView::Input(size_t index) const
{
    if (const auto ret = In(index); ret.has_value()) {
        if (auto result = ret.value().GetIf<T>()) {
            return *result;
        }
    }
    throw std::logic_error("Invalid input");
}

template <typename T>
T ComputerView::Output(size_t index) const
{
    if (const auto ret = Out(index); ret.has_value()) {
        if (auto result = ret.value().GetIf<T>()) {
            return *result;
        }
    }
    throw std::logic_error("Invalid Output");
}

template <typename T>
    requires std::is_constructible_v<PropertyValue, T>
void ComputerView::SetOutput(size_t index, const T& value) const
{
    const auto& address = OutId(index);
    if (!SetValue(address, value))
    {
        std::cout << "Failed to set output value" << std::endl;
    }
}

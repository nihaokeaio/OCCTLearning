//
// Created by ZQD on 26-6-8.
//
#pragma once

#include <string>
#include <vector>

#include "DependencyGraphIds.h"
#include "ValueHandle.h"

struct DGContext;
struct ValueHandle;

struct ComputerView
{
    ComputerView(DGContext& context, const std::vector<ValueId>& inputs, const std::vector<ValueId>& outputs);

    template <typename T>
    T Input(size_t index, const std::string& propertyName) const;

    template <typename T>
    T Output(size_t index, const std::string& propertyName) const;

    template <typename T>
    void SetOutput(size_t index, const std::string& propertyName, const T& value) const;

    [[nodiscard]] ValueId InId(size_t index) const;
    [[nodiscard]] ValueId OutId(size_t index) const;

    [[nodiscard]] ValueHandle& In(size_t index) const;
    [[nodiscard]] ValueHandle& Out(size_t index) const;

private:
    DGContext* m_Context = nullptr;
    const std::vector<ValueId>* m_Inputs = nullptr;
    const std::vector<ValueId>* m_Outputs = nullptr;
};

template <typename T>
T ComputerView::Input(size_t index, const std::string& propertyName) const
{
    return In(index).GetProperty<T>(propertyName);
}

template <typename T>
T ComputerView::Output(size_t index, const std::string& propertyName) const
{
    return Out(index).GetProperty<T>(propertyName);
}

template <typename T>
void ComputerView::SetOutput(size_t index, const std::string& propertyName, const T& value) const
{
    Out(index).SetProperty<T>(propertyName, value);
}

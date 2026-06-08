//
// Created by ZQD on 26-6-8.
//
#pragma once

#include <vector>

#include "DependencyGraphIds.h"

struct DGContext;
struct ValueHandle;

struct ComputerView
{
    ComputerView(DGContext& context, const std::vector<ValueId>& inputs, const std::vector<ValueId>& outputs);

    [[nodiscard]] ValueHandle& In(size_t index) const;
    [[nodiscard]] ValueHandle& Out(size_t index) const;

    [[nodiscard]] ValueId InId(size_t index) const;
    [[nodiscard]] ValueId OutId(size_t index) const;

private:
    DGContext* m_Context = nullptr;
    const std::vector<ValueId>* m_Inputs = nullptr;
    const std::vector<ValueId>* m_Outputs = nullptr;
};

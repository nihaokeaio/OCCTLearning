//
// Created by ZQD on 26-8-14.
//

#pragma once
#include <gp_Pnt.hxx>
#include <numbers>

#include "DependencyGraph/ComputerView.h"

enum class ComputeErrorCode
{
    NONE,
    InvalidInputType,
    InvalidInputNumber,
    InvalidOutputType,
    InvalidOutputNumber
};

struct ComputerResult
{
    bool success;
    ComputeErrorCode code;
};

struct DistanceComputer
{
    void operator()(const ComputerView& view) const
    {
        const auto p0 = view.Input<gp_Pnt>(0);
        const auto p1 = view.Input<gp_Pnt>(1);
        view.SetOutput(0, p0.Distance(p1));
    }
};

struct CircleAreaComputer
{
    void operator()(const ComputerView& view) const
    {
        const auto radiusLength = view.Input<double>(0);
        view.SetOutput(0, std::numbers::pi * radiusLength * radiusLength);
    }
};

struct SumComputer
{
    void operator()(const ComputerView& view) const
    {
        const auto lhs = view.Input<double>(0);
        const auto rhs = view.Input<double>(1);
        view.SetOutput(0, lhs + rhs);
    }
};

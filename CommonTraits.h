//
// Created by ZQD on 26-6-30.
//

#pragma once
#include <tuple>

///类型提取器
template <typename T>
struct FunctionTraits;

///普通函数
template <typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)>
{
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
};

///类成员函数
template <typename C, typename R, typename... Args>
struct FunctionTraits<R(C::*)(Args...)>
{
    using ClassType = C;
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
};

///类成员变量
template <typename C, typename T>
struct FunctionTraits<T C::*>
{
    using MemberType = T;
};

///类成员函数const
template <typename C, typename R, typename... Args>
struct FunctionTraits<R(C::*)(Args...) const>
{
    using ClassType = C;
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
};


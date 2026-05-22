//
// Created by ZQD on 26-5-20.
//

#include "ModernCPPTest.h"
#include <iostream>
#include <map>
#include <type_traits>

// ███╗   ██╗██╗   ██╗██╗     ██╗     ██████╗ ████████╗██████╗
// ████╗  ██║██║   ██║██║     ██║     ██╔══██╗╚══██╔══╝██╔══██╗
// ██╔██╗ ██║██║   ██║██║     ██║     ██████╔╝   ██║   ██████╔╝
// ██║╚██╗██║██║   ██║██║     ██║     ██╔═══╝    ██║   ██╔══██╗
// ██║ ╚████║╚██████╔╝███████╗███████╗██║        ██║   ██║  ██║
// ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚══════╝╚═╝        ╚═╝   ╚═╝  ╚═╝
namespace NULLPTR
{
    void foo(char*);
    void foo(int);

    void test()
    {
        if (std::is_same<decltype(NULL), decltype(0)>::value)
        {
            std::cout << "NULL == 0" << std::endl;
        }
        if (std::is_same<decltype(NULL), decltype((void*)0)>::value)
        {
            std::cout << "NULL == (void*)0" << std::endl;
        }
        if (std::is_same<decltype(NULL), decltype(nullptr)>::value)
        {
            std::cout << "NULL == std::nullptr_t" << std::endl;
        }
    }
}

namespace TEMPLATE
{
    namespace VARIABLE_PARAMETER
    {
        template <typename T>
        void printf1(T args)
        {
            std::cout << args << std::endl;
        }

        template <typename T, typename... Args>
        void printf1(T t0, Args... args)
        {
            std::cout << t0 << std::endl;
            printf1(args...);
        }

        template <typename T, typename... Args>
        void printf2(T t0, Args... args)
        {
            std::cout << t0 << std::endl;
            if constexpr (sizeof...(args) > 0)
                printf2(args...);
        }

        void foo()
        {
            printf2(1, 2, "123", 1.1);
        }

        template <typename KEY, typename VALUE, typename FUNCTION>
        void update(std::map<KEY, VALUE>& m, FUNCTION foo)
        {
            for (auto&& [k,v] : m)
            {
                v = foo(k);
            }
        }

        void foo2()
        {
            std::map<std::string, long long int> m{
                {"a", 1},
                {"b", 2},
                {"c", 3}
            };
            update(m, [](const std::string& key)
            {
                return std::hash<std::string>{}(key);
            });

            for (auto&& [key, value] : m)
            {
                std::cout << key << ": " << value << std::endl;
            }
        }
    }
}


void ModernCPPTest::Test()
{
    TEMPLATE::VARIABLE_PARAMETER::foo2();
}

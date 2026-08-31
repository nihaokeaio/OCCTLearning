//
// Created by ZQD on 26-8-21.
//
#include <iostream>
#include <type_traits>
#include <utility>
#include <gtest/gtest.h>

/// 左值/右值
namespace Version1
{
    template <typename T>
    void fooByValue(T v)
    {
        if constexpr (std::is_same_v<T, int>)
        {
            std::cout << "T = int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int>)
        {
            std::cout << "T = const int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, int&>)
        {
            std::cout << "T = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int&>)
        {
            std::cout << "T = const int&" << std::endl;
        }

        using param = decltype(v);

        if constexpr (std::is_same_v<param, int&>)
        {
            std::cout << "param = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&>)
        {
            std::cout << "param = const int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, int&&>)
        {
            std::cout << "param = int&&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&&>)
        {
            std::cout << "param =const int&&" << std::endl;
        }
        std::cout << std::endl;
    }

    template <typename T>
    void fooByForwardingRef(T&& v)
    {
        if constexpr (std::is_same_v<T, int>)
        {
            std::cout << "T = int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int>)
        {
            std::cout << "T = const int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, int&>)
        {
            std::cout << "T = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int&>)
        {
            std::cout << "T = const int&" << std::endl;
        }

        using param = decltype(v);

        if constexpr (std::is_same_v<param, int&>)
        {
            std::cout << "param = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&>)
        {
            std::cout << "param = const int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, int&&>)
        {
            std::cout << "param = int&&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&&>)
        {
            std::cout << "param =const int&&" << std::endl;
        }
        std::cout << std::endl;
    }

    template <typename T>
    void fooByLValueRef(T& v)
    {
        // 注意T的类型，正常调用不会出现int&&
        if constexpr (std::is_same_v<T, int>)
        {
            std::cout << "T = int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int>)
        {
            std::cout << "T = const int" << std::endl;
        }
        else if constexpr (std::is_same_v<T, int&>)
        {
            std::cout << "T = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const int&>)
        {
            std::cout << "T = const int&" << std::endl;
        }

        using param = decltype(v);

        if constexpr (std::is_same_v<param, int&>)
        {
            std::cout << "param = int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&>)
        {
            std::cout << "param = const int&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, int&&>)
        {
            std::cout << "param = int&&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const int&&>)
        {
            std::cout << "param =const int&&" << std::endl;
        }
        std::cout << std::endl;
    }
}

/// 完美转发
namespace Version2
{
    class A
    {
    public:
        A() = default;
        ~A() = default;

        A(const A&)
        {
            std::cout << "A copy construct" << std::endl;
        }

        A(A&&) noexcept
        {
            std::cout << "A move construct" << std::endl;
        }

        A& operator=(A&& a) noexcept
        {
            std::cout << "A move assign" << std::endl;
            return *this;
        }

        A& operator=(const A&) noexcept
        {
            std::cout << "A copy assign" << std::endl;
            return *this;
        }
    };


    void consume(A value)
    {
        std::cout << "consume\n";
    }

    template <typename P, typename T>
    void fooByForwardingRef(T&& v)
    {
        if constexpr (std::is_same_v<T, P>)
        {
            std::cout << "T = P" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const P>)
        {
            std::cout << "T = const P" << std::endl;
        }
        else if constexpr (std::is_same_v<T, P&>)
        {
            std::cout << "T = P&" << std::endl;
        }
        else if constexpr (std::is_same_v<T, const P&>)
        {
            std::cout << "T = const P&" << std::endl;
        }

        using param = decltype(v);

        if constexpr (std::is_same_v<param, P&>)
        {
            std::cout << "param = P&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const P&>)
        {
            std::cout << "param = const P&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, P&&>)
        {
            std::cout << "param = P&&" << std::endl;
        }
        else if constexpr (std::is_same_v<param, const P&&>)
        {
            std::cout << "param =const P&&" << std::endl;
        }
        std::cout << std::endl;
    }

    template <typename T>
    void passDirectly(T&& v)
    {
        consume(v);
        fooByForwardingRef<A>(v);
    }

    template <typename T>
    void passWithForward(T&& v)
    {
        consume(std::forward<T>(v));
        fooByForwardingRef<A>(std::forward<T>(v));
    }

    template <typename T>
    void passWithMove(T&& v)
    {
        consume(std::move(v));
        fooByForwardingRef<A>(std::move(v));
    }
}


/// 类模板
namespace Version3
{
    /// 主模板
    template <typename T>
    class ValueHolder
    {
    public:
        explicit ValueHolder(T value): m_Value(std::move(value))
        {
        }

        const T& Get() const
        {
            if constexpr (std::is_same_v<T, double>)
            {
                std::cout << "T = double" << std::endl;
            }
            return m_Value;
        }

    private:
        T m_Value;
    };

    /// 全特化
    template <>
    class ValueHolder<int>
    {
    public:
        explicit ValueHolder(int value): m_Value(std::move(value))
        {
        }

        [[nodiscard]] const int& Get() const
        {
            std::cout << "m_Value = " << m_Value << std::endl;
            return m_Value;
        }

    private:
        int m_Value;
    };

    /// 偏特化
    template <typename U>
    class ValueHolder<U*>
    {
    public:
        explicit ValueHolder(U* value): m_Value(value)
        {
        }

        U* const& Get() const
        {
            std::cout << "pointer partial specialization\n";
            return m_Value;
        }

    private:
        U* m_Value;
    };
}

/// 萃取
namespace Version4
{
    template <typename T>
    struct MyRemoveReference
    {
        using type = T;
    };


    template <typename T>
    struct MyRemoveReference<T&>
    {
        using type = T;
    };

    template <typename T>
    struct MyRemoveReference<T&&>
    {
        using type = T;
    };

    template <typename T>
    using MyRemoveReferenceT = typename MyRemoveReference<T>::type;

    template <typename T>
    MyRemoveReferenceT<T>&& Move(T&& v)
    {
        //static_assert(std::is_same_v<const int, U>);
        return static_cast<MyRemoveReferenceT&&>(v);
    }

    template <typename T>
    struct MyRemoveCV
    {
        using Type = T;
    };

    template <typename T>
    struct MyRemoveCV<const T>
    {
        using Type = T;
    };

    template <typename T>
    struct MyRemoveCV<volatile T>
    {
        using Type = T;
    };


    template <typename T>
    struct MyRemoveCV<const volatile T>
    {
        using Type = T;
    };


    /// 去除顶层CV，而不去除底层CV===>顶层：修饰变量本身，int* const p;（p 本身不能变），底层：修饰指向的对象，const int* p;（p 指向的对象不能变）
    template <typename T>
    using MyRemoveCVT = typename MyRemoveCV<T>::Type;
    template <typename T>
    using MyRemoveCVRefT = MyRemoveCVT<MyRemoveReferenceT<T>>;


    template <typename T>
    struct MyIsPointerImpl : std::false_type
    {
    };

    template <typename T>
    struct MyIsPointerImpl<T*> : std::true_type
    {
    };

    // 手动写const.volatile版本，或者使用MyRemoveCVT组合
    // template <typename T>
    // struct MyIsPointerImpl<T* const> : std::true_type
    // {
    // };
    //
    // template <typename T>
    // struct MyIsPointerImpl<T* volatile> : std::true_type
    // {
    // };
    //
    // template <typename T>
    // struct MyIsPointerImpl<T* const volatile> : std::true_type
    // {
    // };
    // template <typename T>
    // inline constexpr bool MyIsPointerImplV = MyIsPointerImpl<T>::value;

    template <typename T>
    struct MyIsPointer : MyIsPointerImpl<MyRemoveCVT<T>>
    {
    };

    template <typename T>
    inline constexpr bool MyIsPointerV = MyIsPointer<T>::value;
}

/// concept概念requires约束的使用
namespace Version5
{
    using namespace Version4;


    template <typename T>
    concept Dereferenceable = requires(T&& v) { *v ; };


    template <typename T>
    concept PointerType = MyIsPointerV<MyRemoveCVRefT<T>> && Dereferenceable<T>;

    template <typename T>
        requires PointerType<T>
    auto& Dereference(T&& pointer)
    {
        if constexpr (std::is_const_v<std::remove_pointer_t<MyRemoveCVRefT<T>>>)
        {
            std::cout << "Dereference Const Type" << std::endl;
        }
        else
        {
            std::cout << "Dereference NonConst Type" << std::endl;
        }
        return *pointer;
    }

    /// 约束是否具有某个成员
    template <typename T>
    concept HasConstGet = requires(const T& object)
    {
        object.Get();
    };

    /// 约束返回值是否符合
    template <typename T>
    concept GetReturnsConstIntRef = HasConstGet<T> && requires(const T& object)
    {
        { object.Get() } -> std::same_as<const int&>;
    };

    /// 约束重载排序，更严格的约束应该放在宽松的后面
    template <typename T>
    void Describe(const T& object)
    {
        std::cout << "generic\n";
    }

    template <typename T>
    void Describe(const T& object) requires HasConstGet<T>
    {
        std::cout << "has const Get\n";
    }

    template <GetReturnsConstIntRef T>
    void Describe(const T& object)
    {
        std::cout << "Get returns const int&\n";
    }
}

/// parameter pack 参数包
namespace Version6
{
    /// 声明参数包
    template <typename... Args>
    constexpr size_t ArgumentCount(Args&&...)
    {
        return sizeof...(Args);
    }

    /// 参数包展开
    template <typename... Args>
    void PrintAll(Args&&... args)
    {
        /// 折叠表达式 ((expression)op...);(...op(expression))
        /// 这里使用了逗号运算符，从左到右依次执行每个表达式，返回最后一个表达式的值
        ((std::cout << std::forward<Args>(args) << " "), ...);
        std::cout << std::endl;
    }

    /// 数值参数包
    /// 一元折叠 右折叠(args - ...)=>(0-(1-2));左折叠(... + args)=>((0-1)-2);=>没有初始值
    /// 二元折叠 右折叠(args - ... - init)=>(0-(1-2));左折叠(init ... + args)=>((0-1)-2);=>注意初始值与...的相对位置
    template <typename... Args>
    auto Sum(Args&&... args)
    {
        return (0 + ... + std::forward<Args>(args));
    }

    /// 二元左折叠
    template <typename... Args>
    auto SubtractLeft(Args&&... args)
    {
        return (0 - ... - std::forward<Args>(args));
    }

    /// 二元右折叠
    template <typename... Args>
    auto SubtractRight(Args&&... args)
    {
        return (std::forward<Args>(args) - ... - 0);
    }

    template <typename Args>
    concept Arithmetic = std::is_arithmetic_v<Version4::MyRemoveCVRefT<Args>>;

    ///template <Arithmetic... Args> 含义为对参数包中的每个参数，使用Arithmetic
    template <Arithmetic... Args>
    auto Add(Args&&... args)
    {
        return (0 + ... + std::forward<Args>(args));
    }

    /// 或者在概念处实现解包约束
    template <typename... Args>
    concept AllArithmetic = (std::is_arithmetic_v<Version4::MyRemoveCVRefT<Args>> && ...);

    //template <AllArithmetic... Args>
    template <typename... Args>
        requires AllArithmetic<Args...>
    auto Added(Args&&... args)
    {
        return (0 + ... + std::forward<Args>(args));
    }
}

namespace Version7
{
    template <typename Tuple, size_t... Index>
    void PrintTupleImpl(const Tuple& tuple, std::index_sequence<Index...>)
    {
        ((std::cout << std::get<Index>(tuple) << " "), ...);
        if constexpr (sizeof...(Index) > 0)
        {
            std::cout << std::endl;
        }
    }

    template <typename... Ts>
    void PrintTuple(const std::tuple<Ts...>& tuple)
    {
        PrintTupleImpl(tuple, std::index_sequence_for<Ts...>{}); // 创建0-N-1的序列
        PrintTupleImpl(tuple, std::make_index_sequence<sizeof...(Ts)>{}); // 创建0-N-1的序列
        PrintTupleImpl(tuple, std::make_index_sequence<std::tuple_size_v<std::tuple<Ts...>>>{});
        //std::tuple_size_v接受一个完整类型
    }

    template <typename Tuple>
    void PrintTuple(const Tuple& tuple)
    {
        PrintTupleImpl(tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    /// fold expression=>对包中的每个参数执行fun(args)
    template <typename Tuple, size_t... Index, typename F>
    void TupleForEachImpl(const Tuple& tuple, std::index_sequence<Index...>, F&& f)
    {
        auto call = [&]<typename Fun,typename T>(Fun& fun, T&& args)
        {
            if constexpr (std::is_invocable_v<Fun&, T>)
            {
                std::invoke(fun, std::forward<T>(args));
            }
        };
        /// 多次调用，应该为左值
        (call(f, std::get<Index>(tuple)), ...);
        //(std::invoke(std::forward<F>(f), std::get<Index>(tuple)), ...);
    }

    template <typename... Tuple, typename F>
    void TupleForEach(const std::tuple<Tuple...>& tuple, F&& f)
    {
        TupleForEachImpl(tuple, std::index_sequence_for<Tuple...>{}, std::forward<F>(f));
    }

    template <typename Tuple, size_t... Index, typename F>
    decltype(auto) TupleApplyImpl(const Tuple& tuple, std::index_sequence<Index...>, F&& f)
    {
        return std::invoke(std::forward<F>(f), (std::get<Index>(tuple))...);
    }

    /// pack expansion：把所有元素展开到某个语法位置
    template <typename... Tuple, typename F>
        requires std::is_invocable_v<F&&, const Tuple&...>
    decltype(auto) TupleApply(const std::tuple<Tuple...>& tuple, F&& f)
    {
        return TupleApplyImpl(tuple, std::index_sequence_for<Tuple...>{}, std::forward<F>(f));
    }
}

namespace Version8
{
    template <typename T>
    struct FunctionTraits : FunctionTraits<decltype(&T::operator())>
    {
    };

    template <typename R, typename... Args>
    struct FunctionTraits<R(Args...)>
    {
        using ReturnType = R;
        using ArgumentTuple = std::tuple<Args...>;
        static constexpr size_t ArgumentCount = sizeof...(Args);

        template <std::size_t I>
        using Argument = std::tuple_element_t<I, ArgumentTuple>;
        static constexpr bool IsNoexcept = false;
    };

    template <typename R, typename... Args>
    struct FunctionTraits<R(Args...) noexcept> : FunctionTraits<R(Args...)>
    {
        static constexpr bool IsNoexcept = true;
    };

    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)> : FunctionTraits<R(Args...)>
    {
    };

    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...) noexcept> : FunctionTraits<R(Args...) noexcept>
    {
    };

    template <typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...)> : FunctionTraits<R(Args...)>
    {
        using ClassType = C;
    };

    template <typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...) noexcept> : FunctionTraits<R(Args...) noexcept>
    {
        using ClassType = C;
    };

    template <typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...) const> : FunctionTraits<R(Args...)>
    {
        using ClassType = C;
    };

    template <typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...) const noexcept> : FunctionTraits<R(Args...) noexcept>
    {
        using ClassType = C;
    };

    double FreeFunction(int, const std::string&)
    {
        return 1.0;
    }

    struct Demo
    {
        double GetNumber(int, const std::string&)
        {
            return 1.0;
        }

        int GetConstNumber(int, const std::string&) const
        {
            return 2;
        }

        double NoexceptFunction(int, const std::string&) noexcept
        {
            return 1.0;
        }
    };

    struct Multiplier
    {
        double operator()(int, const std::string&) const
        {
            return 1.0;
        }

        double operator()(int, const std::string&, float) const
        {
            return 1.0;
        }
    };

    auto lambda = [](int, const std::string&)-> double
    {
        return 1.0;
    };

    auto templateLambda = []<typename T>(T v)-> T
    {
        return v;
    };
};

TEST(ModernCPPTemplateTest, fooByValue)
{
    using namespace Version1;
    int v0 = 3;
    const int v1 = 2;
    fooByValue(v0);
    fooByValue(v1);
    fooByValue(5);
    fooByValue(std::move(v0));
    //结果均为 int
    EXPECT_EQ(1+1, 2);
}

TEST(ModernCPPTemplateTest, fooByforwardingRef)
{
    using namespace Version1;
    int v0 = 3;
    const int v1 = 2;
    fooByForwardingRef(v0);
    fooByForwardingRef(v1);
    fooByForwardingRef(5);
    fooByForwardingRef(std::move(v0));
    fooByForwardingRef(std::move(v1));
    EXPECT_EQ(1+1, 2);
}

TEST(ModernCPPTemplateTest, fooByLValueRef)
{
    using namespace Version1;
    int v0 = 3;
    const int v1 = 2;
    fooByLValueRef(v0);
    fooByLValueRef(v1);
    // fooByLValueRef(5);
    // fooByLValueRef(std::move(v0));
    fooByLValueRef(std::move(v1));
    EXPECT_EQ(1+1, 2);
}

TEST(ModernCPPTemplateTest, MoveAndForward)
{
    using namespace Version2;

    std::cout << "directly with lvalue\n";
    A a1;
    passDirectly(a1); // copy

    std::cout << "directly with rvalue\n";
    passDirectly(A{}); // copy，因为具名参数 v 是左值

    std::cout << "forward with lvalue\n";
    A a2;
    passWithForward(a2); // copy

    std::cout << "forward with rvalue\n";
    passWithForward(A{}); // move

    std::cout << "move with lvalue\n";
    A a3;
    passWithMove(a3); // move
    EXPECT_EQ(1+1, 2);
}

TEST(ModernCPPTemplateTest, ClassTemplate)
{
    using namespace Version3;

    ValueHolder<double> v0(2.0);
    ValueHolder<std::string> v1("hello");
    ValueHolder v2(3);
    int* p0 = new int(1);
    const int p1 = 2;
    ValueHolder v4(p0);
    ValueHolder v5(&p1);
    auto t0 = v4.Get();
    auto t1 = v5.Get();
    static_assert(std::is_same_v<decltype(v2), ValueHolder<int>>);
    EXPECT_DOUBLE_EQ(v0.Get(), 2.0);
    EXPECT_EQ(v1.Get(), "hello");
    EXPECT_EQ(v2.Get(), 3);
    delete p0;
}

TEST(ModernCPPTemplateTest, TypeTraits)
{
    using namespace Version4;
    static_assert(std::is_same_v<int, MyRemoveReferenceT<int>>);
    static_assert(std::is_same_v<int, MyRemoveReferenceT<int&>>);
    static_assert(std::is_same_v<int, MyRemoveReferenceT<int&&>>);
    static_assert(std::is_same_v<const int, MyRemoveReferenceT<const int&&>>);
    int a = 0;
    const int b = 0;
    static_assert(std::is_same_v<int, MyRemoveReferenceT<decltype(a)>>);
    static_assert(std::is_same_v<int, MyRemoveReferenceT<decltype(std::move(a))>>);
    static_assert(std::is_same_v<int, MyRemoveReferenceT<decltype(5)>>);
    static_assert(std::is_same_v<const int, MyRemoveReferenceT<decltype(b)>>);
    static_assert(std::is_same_v<const int, MyRemoveReferenceT<decltype(std::move(b))>>);

    static_assert(std::is_same_v<int&&, decltype(Move(a))>);
    static_assert(std::is_same_v<const int&&, decltype(Move(b))>);


    ///指针测试
    ///EXPECT_TRUE无法识别"<>,"符号，可以添加一个（）做处理，此处为编译期判断，故还是使用static_assert为主
    EXPECT_TRUE(!MyIsPointerV<int>);
    EXPECT_TRUE(MyIsPointerV<int*>);
    EXPECT_TRUE(MyIsPointerV<const int*>);
    EXPECT_TRUE(MyIsPointerV<int* const>);
    EXPECT_TRUE(MyIsPointerV<const int*const>);
    EXPECT_TRUE(MyIsPointerV<const int* volatile>);
    EXPECT_TRUE(MyIsPointerV<const int* const volatile>);

    ///removeCV测试
    int c = 0;
    const int d = 0;
    volatile int e = 0;
    const volatile int f = 0;
    const volatile int& g = c;
    static_assert(std::is_same_v<int, MyRemoveCVT<decltype(c)>>);
    static_assert(std::is_same_v<int, MyRemoveCVT<decltype(d)>>);
    static_assert(std::is_same_v<int, MyRemoveCVT<decltype(e)>>);
    static_assert(std::is_same_v<int, MyRemoveCVT<decltype(f)>>);
    static_assert(std::is_same_v<const volatile int&, MyRemoveCVT<decltype(g)>>);
    static_assert(std::is_same_v<int, MyRemoveCVRefT<decltype(g)>>);
    static_assert(std::is_same_v<int&&, MyRemoveCVT<decltype(std::move(c))>>);
    static_assert(std::is_same_v<const volatile int&&, MyRemoveCVT<decltype(std::move(g))>>);

    // 底层 const 不移除
    static_assert(std::is_same_v<MyRemoveCVT<const int*>, const int*>);
    // 顶层 const 被移除
    static_assert(std::is_same_v<MyRemoveCVT<int* const>, int*>);
    // 引用不是指针
    static_assert(!MyIsPointerV<int*&>);
    // 去掉引用后才是指针
    static_assert(MyIsPointerV<MyRemoveCVRefT<int*&>>);
}


TEST(ModernCPPTemplateTest, ConceptAndRequiresTest)
{
    using namespace Version5;
    static_assert(PointerType<int*>);
    static_assert(PointerType<const int*>);
    static_assert(PointerType<int* const>);
    static_assert(PointerType<int*&>);
    static_assert(!PointerType<int>);
    // 使用requires判断是否可解引
    static_assert(!PointerType<void*>); //void*指针无法解引


    int a = 5;
    int* point = &a;
    const int* constPointer = &a;
    EXPECT_EQ(Dereference(point), 5);
    EXPECT_EQ(Dereference(constPointer), 5);
    static_assert(std::is_same_v<int&, decltype(Dereference(point))>);
    static_assert(std::is_same_v<const int&, decltype(Dereference(constPointer))>);
    //DeReference(a);编译错误


    // 使用requires判断是否具有某个成员函数
    static_assert(HasConstGet<Version3::ValueHolder<int>>);
    static_assert(HasConstGet<Version3::ValueHolder<double*>>);
    static_assert(!HasConstGet<int>);

    // 使用requires判断返回来别是否符合
    static_assert(GetReturnsConstIntRef<Version3::ValueHolder<int>>);
    static_assert(!GetReturnsConstIntRef<Version3::ValueHolder<double>>);

    Describe(5); // generic
    Describe(Version3::ValueHolder<double>{2.0}); // has const Get
    Describe(Version3::ValueHolder<int>{2}); // Get returns const int&
}

TEST(ModernCPPTemplateTest, ParameterPackTest)
{
    using namespace Version6;
    static_assert(ArgumentCount() == 0);
    static_assert(ArgumentCount(1, 2) == 2);
    static_assert(ArgumentCount(1, 2, "hello_world") == 3);

    PrintAll();
    PrintAll(1, 2, "hello_world");
    EXPECT_EQ(Sum(), 0);
    EXPECT_EQ(Sum(1,2,3,4,5), 15);
    EXPECT_DOUBLE_EQ(Sum(1,2.5,3), 6.5);

    EXPECT_EQ(SubtractLeft(), 0);
    EXPECT_EQ(SubtractRight(), 0);
    EXPECT_EQ(SubtractLeft(10,3,2), -15); // (((0-10)-3)-2)
    EXPECT_EQ(SubtractRight(10,3,2), 9); //(10-(3-(2-0)))

    EXPECT_EQ(Add(), 0);
    EXPECT_EQ(Add(1,2,3,4), 10);
    EXPECT_DOUBLE_EQ(Add(1,2,3,4.1,true), 11.1);
    EXPECT_DOUBLE_EQ(Added(1,2,3,4.1,true), 11.1);
}


TEST(ModernCPPTemplateTest, TuplePackTest)
{
    using namespace Version7;
    auto t = std::make_tuple(1, 2.3, "4", "hello_world");
    PrintTuple(t);
    PrintTuple(std::tuple{});
    PrintTuple(std::pair{1, 2}); //重载版本

    int callCount = 0;
    TupleForEach(t, [&callCount](auto&& v) requires requires
    {
        v * 2 ;
    }
    {
        std::cout << v << " ";
        auto k = v * 2;
        ++callCount;
    });
    std::cout << std::endl;
    EXPECT_EQ(callCount, 2);

    callCount = 0;
    TupleApply(t, [&callCount]<typename... T>(T&&... args)
    {
        ((std::cout << std::forward<T>(args) << " "), ...);
        ++callCount;
    });
    std::cout << std::endl;
    EXPECT_EQ(callCount, 1);
}

TEST(ModernCPPTemplateTest, FunTraitsTest)
{
    using namespace Version8;

    // 普通函数与函数指针
    using FunctionType = decltype(FreeFunction);
    using Traits = FunctionTraits<FunctionType>;
    static_assert(Traits::ArgumentCount == 2);
    static_assert(std::is_same_v<double, Traits::ReturnType>);
    static_assert(std::is_same_v<int, Traits::Argument<0>>);
    static_assert(std::is_same_v<const std::string&, Traits::Argument<1>>);

    // 类成员函数
    using ClassFunctionType = decltype(&Demo::GetNumber);
    using ClassTraits = FunctionTraits<ClassFunctionType>;
    static_assert(std::is_same_v<double, ClassTraits::ReturnType>);
    static_assert(std::is_same_v<int, ClassTraits::Argument<0>>);
    static_assert(std::is_same_v<const std::string&, ClassTraits::Argument<1>>);
    static_assert(std::is_same_v<Demo, ClassTraits::ClassType>);
    static_assert(!ClassTraits::IsNoexcept);


    // 类成员函数const版本
    using ClassConstFunctionType = decltype(&Demo::GetConstNumber);
    using ClassConstTraits = FunctionTraits<ClassConstFunctionType>;
    static_assert(std::is_same_v<int, ClassConstTraits::ReturnType>);

    // 函数对象，operator()
    using LambdaTraits = FunctionTraits<double(Multiplier::*)(int, const std::string&, float) const>;
    static_assert(std::is_same_v<double, LambdaTraits::ReturnType>);
    static_assert(std::is_same_v<Multiplier, LambdaTraits::ClassType>);

    // 匿名函数，本质就是一个匿名的函数对象
    using lambdaType = decltype(lambda);
    using UnNamedLambdaTraits = FunctionTraits<lambdaType>;
    static_assert(std::is_same_v<double, UnNamedLambdaTraits::ReturnType>);
    static_assert(std::is_same_v<lambdaType, UnNamedLambdaTraits::ClassType>);


    // 类成员函数noexcept
    using ClassNoExceptFunctionType = decltype(&Demo::NoexceptFunction);
    using ClassNoExceptTraits = FunctionTraits<ClassNoExceptFunctionType>;
    static_assert(std::is_same_v<double, ClassNoExceptTraits::ReturnType>);
    static_assert(std::is_same_v<int, ClassNoExceptTraits::Argument<0>>);
    static_assert(std::is_same_v<const std::string&, ClassNoExceptTraits::Argument<1>>);
    static_assert(std::is_same_v<Demo, ClassNoExceptTraits::ClassType>);
    static_assert(!ClassTraits::IsNoexcept);

    // 泛型lambda
    using templateLambdaType = decltype(templateLambda);
    using templateLambdaOperatorType = decltype(&templateLambdaType::operator()<int>);
    using templateLambdaTraits = FunctionTraits<templateLambdaOperatorType>;
    static_assert(std::is_same_v<int, templateLambdaTraits::ReturnType>);
    static_assert(std::is_same_v<templateLambdaType, templateLambdaTraits::ClassType>);
}

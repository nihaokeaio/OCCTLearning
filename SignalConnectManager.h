//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
#include "CommonTraits.h"
#include <functional>
#include <iostream>
#include <vector>
#include <concepts>

template <typename T>
struct TypeDumper;

///设计一个信号类
namespace MiniSignal
{
    class Connection
    {
    public:
        explicit Connection(const std::function<void()>& f): m_DisConnectFun(f)
        {
        }

        void DisConnect()
        {
            if (!m_DisConnectFun)
                return;
            std::cout << "Connection is disConnect\n";
            m_DisConnectFun();
            m_DisConnectFun = nullptr;
        }

        // 检查是否已断开
        [[nodiscard]] bool IsDisconnected() const
        {
            return !m_DisConnectFun;
        }

    private:
        std::function<void()> m_DisConnectFun;
    };

    class ScopeConnection
    {
    public:
        explicit ScopeConnection(const std::shared_ptr<Connection>& connection): m_Connection(connection)
        {
        }

        /// 不可拷贝
        ScopeConnection(const ScopeConnection&) = delete;
        ScopeConnection& operator=(const ScopeConnection&) = delete;

        ~ScopeConnection()
        {
            if (m_Connection)
            {
                m_Connection->DisConnect();
            }
        }

    private:
        std::shared_ptr<Connection> m_Connection;
    };

    class Trackable
    {
    public:
        Trackable() = default;
        Trackable(const Trackable& trackable) = delete;
        Trackable& operator=(const Trackable& trackable) = delete;

        ~Trackable()
        {
            // 析构时自动断开所有连接
            for (auto& conn : m_Connections)
            {
                if (conn) conn->DisConnect();
            }
        }

        void addConnection(const std::shared_ptr<Connection>& conn)
        {
            m_Connections.push_back(conn);
        }

    private:
        std::vector<std::shared_ptr<Connection>> m_Connections;
    };


    template <typename... Args>
    class Signal
    {
    public:
        Signal(): m_State(std::make_shared<State>())
        {
        }

        struct Slot
        {
            std::function<void(Args...)> fun;
            bool connected = true;
        };

        struct State
        {
            std::vector<std::shared_ptr<Slot>> slots{};
        };

        using Signature = void(Args...);
        using ArgsTuple = std::tuple<Args...>;

        ///匿名函数
        std::shared_ptr<Connection> connect(const std::function<void(Args... args)>& f)
        {
            auto s = std::make_shared<Slot>();
            s->fun = f;
            m_State->slots.push_back(s);
            std::weak_ptr<State> weakState = m_State;
            auto conn = std::make_shared<Connection>([weakState,s]()
            {
                if (auto state = weakState.lock())
                {
                    //state->slots.erase(std::remove(state->slots.begin(), state->slots.end(), s), state->slots.end());
                    s->connected = false;
                }
            });
            return conn;
        }

        void emit(Args... args)
        {
            auto slots = m_State->slots;
            for (const auto& s : slots)
            {
                ///保护m_State->slots以防止其在 s->fun(args...)中被修改
                if (s->connected)
                    s->fun(args...);
            }
            cleanUp();
        }

    private:
        void cleanUp()
        {
            std::erase_if(m_State->slots, [](const std::shared_ptr<Slot>& slot)
            {
                return !slot->connected;
            });
        }

    private:
        std::shared_ptr<State> m_State;
    };

    template <typename Callable, typename Object, typename Tuple>
    struct IsInvocableWithTuple;

    template <typename Callable, typename Object, typename... Args>
    struct IsInvocableWithTuple<Callable, Object, std::tuple<Args...>>
    {
        static constexpr bool value = std::is_invocable_v<Callable, Object, Args...>;
    };

    template <typename Callable, typename Object, typename... Args>
    concept Invocable = IsInvocableWithTuple<Callable, Object, Args...>::value;

    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
        requires Invocable<SlotType, Receiver*, typename FunctionTraits<SignalType>::MemberType::ArgsTuple>
    ScopeConnection connectScope(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        return ScopeConnection(connect(sender, signal, receiver, slot));
    }

    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    std::enable_if_t<IsInvocableWithTuple<SlotType, Receiver*, typename FunctionTraits<
                                              SignalType>::MemberType::ArgsTuple>::value, std::shared_ptr<Connection>>
    connect(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        //1. 提取类型
        using SignalTraits = FunctionTraits<SignalType>;

        using SignalClass = typename SignalTraits::MemberType;
        using SignalArgs = typename SignalClass::ArgsTuple;
        //2. 编译检查
        static_assert(IsInvocableWithTuple<SlotType, Receiver*, SignalArgs>::value,
                      "slot cannot be invoked with signal arguments");
        static_assert(std::is_base_of_v<Trackable, Receiver>, "Receiver must be derived from Trackable");
        // 3. 真正连接（调用你已有的 Signal::connect）
        auto conn = (sender->*signal).connect([receiver,slot]<typename... T0>(T0&&... args)
        {
            std::invoke(slot, receiver, std::forward<T0>(args)...);
        });
        static_cast<Trackable*>(receiver)->addConnection(conn);
        return conn;
    }

    template <typename Callable, typename Tuple>
    struct IsInvocableWithTupleLambda;

    template <typename Callable, typename... Args>
    struct IsInvocableWithTupleLambda<Callable, std::tuple<Args...>>
    {
        static constexpr bool value = std::is_invocable_v<Callable, Args...>;
    };

    template <typename Callable, typename... Args>
    concept InvocableLambda = IsInvocableWithTupleLambda<Callable, Args...>::value;

    /// 槽函数版本
    template <typename Sender, typename SignalType, typename SlotType>
        requires InvocableLambda<SlotType, typename FunctionTraits<SignalType>::MemberType::ArgsTuple>
    ScopeConnection connectScope(Sender* sender, SignalType signal, SlotType slot)
    {
        return ScopeConnection(connect(sender, signal, slot));
    }

    template <typename Sender, typename SignalType, typename SlotType>
    std::enable_if_t<IsInvocableWithTupleLambda<SlotType, typename FunctionTraits<
                                                    SignalType>::MemberType::ArgsTuple>::value, std::shared_ptr<
                         Connection>>
    connect(Sender* sender, SignalType signal, SlotType slot)
    {
        auto conn = (sender->*signal).connect([slot=std::move(slot)]<typename... T0>(T0&&... args) mutable
        {
            std::invoke(slot, std::forward<T0>(args)...);
        });
        return conn;
    }


    ///A并不一定需要继承Trackable
    class A : public Trackable
    {
    public:
        Signal<double> sig;
    };

    class B : public Trackable
    {
    public:
        void onSig(int x)
        {
            std::cout << "B::onSig " << x << "\n";
        }
    };
}

class Player
{
public:
    void onDamage(int dmg)
    {
        std::cout << "Player got " << dmg << "\n";
    }
};


class SignalConnectManager
{
public:
    SignalConnectManager()
    {
        test10();
    }

    void test10()
    {
        ///测试槽函数中断开链接的情况
        MiniSignal::A a;
        MiniSignal::B b;
        auto conn = MiniSignal::connect(&a, &MiniSignal::A::sig, &b, &MiniSignal::B::onSig);
        auto lambdaConn = MiniSignal::connectScope(&a, &MiniSignal::A::sig, [conn](int dmg)
        {
            conn->DisConnect();
        });
        a.sig.emit(10);
        ///此时a的连接应该已经断开了
        a.sig.emit(10);
    }
};



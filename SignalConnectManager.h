//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
#include "CommonTraits.h"
#include <functional>
#include <iostream>
#include <vector>

template <typename T>
struct TypeDumper;

///设计一个信号类
namespace Version1
{
    template <typename... Args>
    class Signal
    {
        using Slot = std::function<void(Args...)>;

    public:
        void connect(Slot slot)
        {
            slots.push_back(slot);
        }

        void emit(Args... args)
        {
            for (auto& s : slots)
            {
                s(args...);
            }
        }

    private:
        std::vector<Slot> slots;
    };

    ///问题：当s指向的对象被析构时，s(args...)会崩溃，考虑使用weak_ptr
}

namespace Version2
{
    template <typename... Args>
    class Signal
    {
        struct Slot
        {
            std::function<void(Args...)> fun;
            std::weak_ptr<void> object;
        };

    public:
        template <class T>
        void connect(const std::shared_ptr<T>& object, void (T::*method)(Args... args))
        {
            Slot s;
            std::weak_ptr<T> weakObject = object;
            s.object = object;
            s.fun = [weakObject,method](Args... args)
            {
                if (auto obj = weakObject.lock())
                {
                    (obj.get()->*method)(args...);
                }
            };
            slots.push_back(s);
        }

        void emit(Args... args)
        {
            for (auto it = slots.begin(); it != slots.end();)
            {
                if (it->object.expired())
                {
                    it = slots.erase(it);
                }
                else
                {
                    it->fun(args...);
                    ++it;
                }
            }
        }

    private:
        std::vector<Slot> slots;
    };

    ///问题：如何断开连接呢
}

namespace Version3
{
    template <typename... Args>
    struct SlotBase
    {
        virtual ~SlotBase() = default;
        virtual bool isAlive() =0;

    public:
        std::function<void(Args...)> fun;
        std::weak_ptr<void> object;
    };

    template <typename... Args>
    struct MemberSlot : SlotBase<Args...>
    {
        bool isAlive() override
        {
            return !object.expired();
        }
    };

    template <typename... Args>
    struct LambdaSlot : SlotBase<Args...>
    {
        bool isAlive() override
        {
            return true;
        }
    };


    class Connection
    {
    public:
        explicit Connection(const std::function<void()>& f): disConnectFun(f)
        {
        }

        void disConnect() const
        {
            if (disConnectFun)
            {
                disConnectFun();
            }
        }

    private:
        std::function<void()> disConnectFun;
    };


    template <typename... Args>
    class Signal
    {
    public:
        ///类成员函数
        template <class T>
        Connection connect(const std::shared_ptr<T>& object, void (T::*method)(Args... args))
        {
            auto s = std::make_shared<MemberSlot<Args...>>();
            std::weak_ptr<T> weakObject = object;
            s->object = object;
            s->fun = [weakObject,method](Args... args)
            {
                if (auto obj = weakObject.lock())
                {
                    (obj.get()->*method)(args...);
                }
            };
            slots.push_back(s);
            return Connection([this,s]()
            {
                slots.erase(std::remove(slots.begin(), slots.end(), s), slots.end());
            });
        }

        ///匿名函数
        Connection connect(const std::function<void(Args... args)>& f)
        {
            auto s = std::make_shared<LambdaSlot<Args...>>();
            s->fun = f;
            slots.push_back(s);
            return Connection([this,s]()
            {
                slots.erase(std::remove(slots.begin(), slots.end(), s), slots.end());
            });
        }

        void emit(Args... args)
        {
            for (auto it = slots.begin(); it != slots.end();)
            {
                if (!(*it)->isAlive())
                {
                    it = slots.erase(it);
                }
                else
                {
                    (*it)->fun(args...);
                    ++it;
                }
            }
        }

    private:
        std::vector<std::shared_ptr<SlotBase<Args...>>> slots{};
    };
}

namespace Version4
{
    template <typename... Args>
    struct SlotBase
    {
        virtual ~SlotBase() = default;
        virtual bool isAlive() =0;

    public:
        std::function<void(Args...)> fun;
        std::weak_ptr<void> object;
    };

    template <typename... Args>
    struct MemberSlot : SlotBase<Args...>
    {
        bool isAlive() override
        {
            return !object.expired();
        }
    };

    template <typename... Args>
    struct LambdaSlot : SlotBase<Args...>
    {
        bool isAlive() override
        {
            return true;
        }
    };


    class Connection
    {
    public:
        explicit Connection(const std::function<void()>& f): disConnectFun(f)
        {
        }

        void disConnect() const
        {
            if (disConnectFun)
            {
                disConnectFun();
            }
        }

    private:
        std::function<void()> disConnectFun;
    };


    template <typename... Args>
    class Signal
    {
    public:
        using Signature = void(Args...);
        using ArgsTuple = std::tuple<Args...>;
        ///类成员函数
        template <class T>
        Connection connect(const std::shared_ptr<T>& object, void (T::*method)(Args... args))
        {
            auto s = std::make_shared<MemberSlot<Args...>>();
            std::weak_ptr<T> weakObject = object;
            s->object = object;
            s->fun = [weakObject,method](Args... args)
            {
                if (auto obj = weakObject.lock())
                {
                    (obj.get()->*method)(args...);
                }
            };
            slots.push_back(s);
            return Connection([this,s]()
            {
                slots.erase(std::remove(slots.begin(), slots.end(), s), slots.end());
            });
        }

        ///匿名函数
        Connection connect(const std::function<void(Args... args)>& f)
        {
            auto s = std::make_shared<LambdaSlot<Args...>>();
            s->fun = f;
            slots.push_back(s);
            return Connection([this,s]()
            {
                slots.erase(std::remove(slots.begin(), slots.end(), s), slots.end());
            });
        }

        void emit(Args... args)
        {
            for (auto it = slots.begin(); it != slots.end();)
            {
                if (!(*it)->isAlive())
                {
                    it = slots.erase(it);
                }
                else
                {
                    (*it)->fun(args...);
                    ++it;
                }
            }
        }

    private:
        std::vector<std::shared_ptr<SlotBase<Args...>>> slots{};
    };

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

    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    Connection connect(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        //1. 提取类型
        using SignalTraits = FunctionTraits<SignalType>;
        using SlotTraits = FunctionTraits<SlotType>;

        using SignalClass = typename SignalTraits::MemberType;
        using SignalArgs = typename SignalClass::ArgsTuple;
        using ReceiverArgs = typename SlotTraits::ArgsTuple;

        ///类型检查器
        //TypeDumper<typename SlotTraits::ArgsTuple> dump1;

        //2. 编译检查
        static_assert(std::is_same_v<SignalArgs, ReceiverArgs>, "Signal and Slot arguments must match!");

        // 3. 真正连接（调用你已有的 Signal::connect）
        return (sender->*signal).connect([receiver,slot](auto&&... args)
        {
            (receiver->*slot)(std::forward<decltype(args)>(args)...);
        });
    }

    class A
    {
    public:
        Signal<int> sig;
    };

    class B
    {
    public:
        void onSig(int x)
        {
            std::cout << "B::onSig " << x << "\n";
        }
    };
}


namespace Version5
{
    using Connection = Version4::Connection;

    class Trackable
    {
    public:
        ~Trackable()
        {
            // 析构时自动断开所有连接
            for (auto& conn : connections)
            {
                if (conn) conn->disConnect();
            }
        }

        void addConnection(const std::shared_ptr<Connection>& conn)
        {
            connections.push_back(conn);
        }

    private:
        std::vector<std::shared_ptr<Connection>> connections;
    };


    template <typename... Args>
    class Signal
    {
    public:
        struct Slot
        {
            std::function<void(Args...)> fun;
            Connection* connection;
        };

        using Signature = void(Args...);
        using ArgsTuple = std::tuple<Args...>;

        ///匿名函数
        std::shared_ptr<Connection> connect(const std::function<void(Args... args)>& f)
        {
            auto s = std::make_shared<Slot>();
            s->fun = f;
            slots.push_back(s);
            return std::make_shared<Connection>([this,s]()
            {
                slots.erase(std::remove(slots.begin(), slots.end(), s), slots.end());
            });
        }

        void emit(Args... args)
        {
            for (auto s : slots)
            {
                s->fun(args...);
            }
        }

    private:
        std::vector<std::shared_ptr<Slot>> slots{};
    };

    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    std::shared_ptr<Connection> connect(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        //1. 提取类型
        using SignalTraits = FunctionTraits<SignalType>;
        using SlotTraits = FunctionTraits<SlotType>;

        using SignalClass = typename SignalTraits::MemberType;
        using SignalArgs = typename SignalClass::ArgsTuple;
        using ReceiverArgs = typename SlotTraits::ArgsTuple;

        ///类型检查器
        //TypeDumper<typename SlotTraits::ArgsTuple> dump1;

        //2. 编译检查
        static_assert(std::is_same_v<SignalArgs, ReceiverArgs>, "Signal and Slot arguments must match!");
        static_assert(std::is_base_of_v<Trackable, Receiver>, "Receiver must be derived from Trackable");
        // 3. 真正连接（调用你已有的 Signal::connect）
        auto conn = (sender->*signal).connect([receiver,slot](auto&&... args)
        {
            (receiver->*slot)(std::forward<decltype(args)>(args)...);
        });
        static_cast<Trackable*>(receiver)->addConnection(conn);
        return conn;
    }

    ///A并不一定需要继承Trackable
    class A : public Trackable
    {
    public:
        Signal<int> sig;
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


namespace Version6
{
    using Connection = Version5::Connection;
    using Trackable = Version5::Trackable;


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
                    state->slots.erase(std::remove(state->slots.begin(), state->slots.end(), s), state->slots.end());
                }
            });
            return conn;
        }

        void emit(Args... args)
        {
            for (const auto& s : m_State->slots)
            {
                s->fun(args...);
            }
        }

    private:
        std::shared_ptr<State> m_State;
    };


    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    std::shared_ptr<Connection> connect(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        //1. 提取类型
        using SignalTraits = FunctionTraits<SignalType>;
        using SlotTraits = FunctionTraits<SlotType>;

        using SignalClass = typename SignalTraits::MemberType;
        using SignalArgs = typename SignalClass::ArgsTuple;
        using ReceiverArgs = typename SlotTraits::ArgsTuple;

        ///类型检查器
        //TypeDumper<typename SlotTraits::ArgsTuple> dump1;

        //2. 编译检查
        static_assert(std::is_same_v<SignalArgs, ReceiverArgs>, "Signal and Slot arguments must match!");
        static_assert(std::is_base_of_v<Trackable, Receiver>, "Receiver must be derived from Trackable");
        // 3. 真正连接（调用你已有的 Signal::connect）
        auto conn = (sender->*signal).connect([receiver,slot](auto&&... args)
        {
            (receiver->*slot)(std::forward<decltype(args)>(args)...);
        });
        static_cast<Trackable*>(receiver)->addConnection(conn);
        return conn;
    }

    ///A并不一定需要继承Trackable
    class A : public Trackable
    {
    public:
        Signal<int> sig;
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

namespace Version7
{
    class Connection
    {
    public:
        explicit Connection(const std::function<void()>& f): disConnectFun(f)
        {
        }

        void disConnect()
        {
            if (!disConnectFun)
                return;
            std::cout << "Connection is disConnect\n";
            disConnectFun();
            disConnectFun = nullptr;
        }

        // 检查是否已断开
        bool isDisconnected() const
        {
            return !disConnectFun;
        }

    private:
        std::function<void()> disConnectFun;
    };

    class ConnectionScope
    {
    public:
        explicit ConnectionScope(const std::shared_ptr<Connection>& connection): m_Connection(connection)
        {
        }

        ~ConnectionScope()
        {
            if (m_Connection)
            {
                m_Connection->disConnect();
            }
        }

    private:
        std::shared_ptr<Connection> m_Connection;
    };

    class Trackable
    {
    public:
        ~Trackable()
        {
            // 析构时自动断开所有连接
            for (auto& conn : connections)
            {
                if (conn) conn->disConnect();
            }
        }

        void addConnection(const std::shared_ptr<Connection>& conn)
        {
            connections.push_back(conn);
        }

    private:
        std::vector<std::shared_ptr<Connection>> connections;
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
                    state->slots.erase(std::remove(state->slots.begin(), state->slots.end(), s), state->slots.end());
                }
            });
            return conn;
        }

        void emit(Args... args)
        {
            for (const auto& s : m_State->slots)
            {
                s->fun(args...);
            }
        }

    private:
        std::shared_ptr<State> m_State;
    };


    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    std::shared_ptr<Connection> connect(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        //1. 提取类型
        using SignalTraits = FunctionTraits<SignalType>;
        using SlotTraits = FunctionTraits<SlotType>;

        using SignalClass = typename SignalTraits::MemberType;
        using SignalArgs = typename SignalClass::ArgsTuple;
        using ReceiverArgs = typename SlotTraits::ArgsTuple;

        ///类型检查器
        //TypeDumper<typename SlotTraits::ArgsTuple> dump1;

        //2. 编译检查
        static_assert(std::is_same_v<SignalArgs, ReceiverArgs>, "Signal and Slot arguments must match!");
        static_assert(std::is_base_of_v<Trackable, Receiver>, "Receiver must be derived from Trackable");
        // 3. 真正连接（调用你已有的 Signal::connect）
        auto conn = (sender->*signal).connect([receiver,slot](auto&&... args)
        {
            (receiver->*slot)(std::forward<decltype(args)>(args)...);
        });
        static_cast<Trackable*>(receiver)->addConnection(conn);
        return conn;
    }

    template <typename Sender, typename SignalType, typename Receiver, typename SlotType>
    std::shared_ptr<ConnectionScope> connectScope(Sender* sender, SignalType signal, Receiver* receiver, SlotType slot)
    {
        return std::make_shared<ConnectionScope>(connect(sender, signal, receiver, slot));
    }

    ///A并不一定需要继承Trackable
    class A : public Trackable
    {
    public:
        Signal<int> sig;
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
        test7();
    }

    void test2()
    {
        Version2::Signal<int> signalV2;
        {
            auto p = std::make_shared<Player>();
            signalV2.connect(p, &Player::onDamage);
            signalV2.emit(2); // 会调用
        }
        signalV2.emit(3); // p 已析构，不应调用，并清理 slot
    }

    void test3()
    {
        auto p = std::make_shared<Player>();
        Version3::Signal<int> signalV3;
        auto c0 = signalV3.connect(p, &Player::onDamage);
        auto c1 = signalV3.connect([](int dmg)
        {
            std::cout << "lambda got " << dmg << "\n";
        });

        signalV3.emit(100);
        c0.disConnect();
    }

    void test4()
    {
        Version4::A a;
        Version4::B b;

        auto c = Version4::connect(&a, &Version4::A::sig, &b, &Version4::B::onSig);

        a.sig.emit(10);
        c.disConnect();
    }

    void test5()
    {
        ///测试b对象死亡情况
        Version5::A a;
        std::shared_ptr<Version5::Connection> conn = nullptr;
        {
            Version5::B b;
            conn = Version5::connect(&a, &Version5::A::sig, &b, &Version5::B::onSig);
            a.sig.emit(10);
        }
        a.sig.emit(10);
        conn->disConnect();
    }

    void test6()
    {
        ///测试a对象死亡情况
        Version6::B b;
        std::shared_ptr<Version6::Connection> conn = nullptr;
        {
            Version6::A a;
            conn = Version6::connect(&a, &Version6::A::sig, &b, &Version6::B::onSig);
            a.sig.emit(10);
        }
        conn->disConnect();
    }

    void test7()
    {
        ///测试a对象死亡情况
        Version7::B b;
        {
            Version7::A a;
            auto connScope = Version7::connectScope(&a, &Version7::A::sig, &b, &Version7::B::onSig);
            a.sig.emit(10);
        }
    }
};

static SignalConnectManager signal_connect_manager;




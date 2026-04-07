//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
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
            s.object = object;
            s.fun = [object,method](Args... args)
            {
                (object.get()->*method)(args...);
            };
            slots.push_back(s);
        }

        void emit(Args... args)
        {
            for (auto it = slots.begin(); it != slots.end();)
            {
                if (it->object.expired())
                {
                    slots.erase(it);
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
            s->object = object;
            s->fun = [object,method](Args... args)
            {
                (object.get()->*method)(args...);
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
                    slots.erase(it);
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
            s->object = object;
            s->fun = [object,method](Args... args)
            {
                (object.get()->*method)(args...);
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
                    slots.erase(it);
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
        test4();
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
};

static SignalConnectManager signal_connect_manager;




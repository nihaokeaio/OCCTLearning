//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
#include <functional>
#include <iostream>
#include <vector>

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
        test();
    }

    void test()
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
};

static SignalConnectManager signal_connect_manager;




//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
#include "SignalConnectManager.h"
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_set>
#include <utility>
#include <concepts>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
namespace MiniObjectTree
{
    class Object;

    template <class T>
    concept DerivedFrom = std::derived_from<T, Object>;

    class LifetimeToken
    {
    };

    template <DerivedFrom T>
    class ObjectPtr;

    class Object : public MiniSignal::Trackable
    {
    public:
        explicit Object(std::string name): m_Name(std::move(name)), m_LifetimeToken(std::make_shared<LifetimeToken>())
        {
        }

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) = delete;
        Object& operator=(Object&&) = delete;

        [[nodiscard]] Object* GetParent() const
        {
            return m_Parent;
        }


        [[nodiscard]] std::size_t ChildCount() const noexcept
        {
            return m_Children.size();
        }

        [[nodiscard]] bool HasChild(Object* child) const noexcept
        {
            return m_Children.contains(child);
        }
        ~Object() override
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
            m_LifetimeToken.reset();
            DisAllConnect();
            m_DestroySignal.emit(this);
            // 析构时父对象必须已经为null
            assert(m_Parent == nullptr);
            for (const auto& c : m_Children | std::views::keys)
            {
                c->m_Parent = nullptr;
            }
            m_Children.clear();
            std::cout << "[ " << m_Name << " ]" << " Destroyed End!" << std::endl;
        }

        std::unique_ptr<Object> DetachChild(Object* children)
        {
            if (m_Children.contains(children))
            {
                auto child = std::move(m_Children.at(children));
                m_Children.erase(children);
                child->m_Parent = nullptr;
                return child;
            }
            return nullptr;
        }

        void Reparent(Object* newParent)
        {
            if (newParent == m_Parent)
                return;
            assert(newParent);
            auto c = m_Parent->DetachChild(this);
            // ReSharper disable once CppDFANullDereference
            newParent->AttachChild(std::move(c));
        }

        Object* AttachChild(std::unique_ptr<Object>&& children)
        {
            for (auto ancestor = this; ancestor; ancestor = ancestor->GetParent())
            {
                // 如果children是自己的父对象，就会形成环，这样情况应该被禁止
                if (ancestor == children.get())
                {
                    throw std::logic_error("An object cannot become a child of itself or its descendant");
                }
            }
            children->m_Parent = this;
            const auto rowPtr = children.get();
            m_Children.insert({children.get(), std::move(children)});
            return rowPtr;
        }

    public:
        std::string m_Name;
        MiniSignal::Signal<Object*> m_DestroySignal;

    private:
        std::unordered_map<Object*, std::unique_ptr<Object>> m_Children;
        Object* m_Parent = nullptr;
        std::shared_ptr<LifetimeToken> m_LifetimeToken;

        template <DerivedFrom T>
        friend class ObjectPtr;
    };


    template <DerivedFrom T>
    class ObjectPtr
    {
    public:
        ObjectPtr() = default;

        explicit ObjectPtr(std::nullptr_t) noexcept
        {
        }

        explicit ObjectPtr(T* object): m_Pointer(object)
        {
            if (object)
                m_LifetimeToken = object->m_LifetimeToken;
        }

        [[nodiscard]] T* Get() const noexcept
        {
            if (m_LifetimeToken.expired())
                return nullptr;
            return m_Pointer;
        }

        [[nodiscard]] bool IsNull() const noexcept
        {
            return Get() == nullptr;
        }

        explicit operator bool() const noexcept
        {
            return Get() != nullptr;
        }

        T* operator->() const noexcept
        {
            T* object = Get();
            assert(object != nullptr);
            return object;
        }

        void Reset()
        {
            m_Pointer = nullptr;
            m_LifetimeToken.reset();
        }

    private:
        T* m_Pointer = nullptr;
        std::weak_ptr<LifetimeToken> m_LifetimeToken;
    };

    class Sender
    {
    public:
        MiniSignal::Signal<int, std::string> m_Signal;
    };

    class Receiver : public Object
    {
    public:
        explicit Receiver(const std::string& name): Object(name)
        {
        }

        void OnMessage(const int num, const std::string& message)
        {
            std::cout << "num = " << num << " message = " << message << std::endl;
        }
    };
}


class ObjectTreeManager
{
public:
    ObjectTreeManager()
    {
        test6();
    }

    void test6()
    {
        using namespace MiniObjectTree;
        using namespace MiniSignal;
        auto root = std::make_unique<Object>("root");
        Sender sender;
        auto receiverOwner = std::make_unique<Receiver>("receiver");
        ObjectPtr receiver(receiverOwner.get());
        root->AttachChild(std::move(receiverOwner));
        connect(&sender, &Sender::m_Signal, receiver.Get(), &Receiver::OnMessage);
        bool destroyNotified = false;
        connect(receiver.Get(), &Receiver::m_DestroySignal, [&](const Object* object)
        {
            destroyNotified = true;
            // Destroyed 发出之前 ObjectPtr 应当已经失效
            assert(!receiver);
            std::cout << object->m_Name << " OnDestroy!\n";
        });
        connect(root.get(), &Object::m_DestroySignal, [&](const Object* object)
        {
            destroyNotified = true;
            // Destroyed 发出之前 ObjectPtr 应当已经失效
            assert(!root);
            std::cout << object->m_Name << " OnDestroy!\n";
        });

        sender.m_Signal.emit(5, "start emit");
        root.reset();
        assert(!receiver);
        sender.m_Signal.emit(2, "end emit");
    }
};




//
// Created by ZQD on 26-4-7.
//
/// 仿写QT的信号槽机制


#pragma once
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

namespace Version1
{
    class Object
    {
    public:
        explicit Object(std::string name, Object* parent = nullptr): m_Name(std::move(name)), m_Parent(parent)
        {
            if (m_Parent)
            {
                m_Parent->AddChildren(this);
            }
        }

        [[nodiscard]] Object* GetParent() const
        {
            return m_Parent;
        }

        bool RemoveChildren(Object* children)
        {
            if (const auto iter = m_Children.find(children); iter != m_Children.end())
            {
                m_Children.erase(iter);
                return true;
            }
            return false;
        }

        void AddChildren(Object* children)
        {
            m_Children.insert(children);
        }

        ~Object()
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
            // 从父对象列表中移除
            if (const auto parent = GetParent())
            {
                parent->RemoveChildren(this);
                m_Parent = nullptr;
            }
            // 递归删除子对象
            while (!m_Children.empty())
            {
                const auto iter = m_Children.begin();
                Object* child = *iter;

                // 先解除父子关系，再销毁子对象。这样子对象析构时
                // 不会回头修改当前正在遍历的 m_Children。
                m_Children.erase(iter);
                child->m_Parent = nullptr;
                delete child;
            }
            std::cout << "[ " << m_Name << " ]" << " Destroyed End!" << std::endl;
        }

    private:
        std::unordered_set<Object*> m_Children;
        std::string m_Name;
        Object* m_Parent = nullptr;
    };
}

namespace Version2
{
    class Object
    {
    public:
        explicit Object(std::string name, Object* parent = nullptr): m_Name(std::move(name)), m_Parent(parent)
        {
            if (m_Parent)
            {
                m_Parent->m_Children.insert(this);
            }
        }

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        [[nodiscard]] Object* GetParent() const
        {
            return m_Parent;
        }

        void SetParent(Object* parent)
        {
            if (m_Parent == parent)
                return;
            // 先从已有的父对象中移除
            if (m_Parent)
            {
                m_Parent->RemoveChildren(this);
            }
            // 加入新的父对象
            m_Parent = parent;
            m_Parent->m_Children.insert(this);
        }

        void AddChildren(Object* children)
        {
            if (children->m_Parent == this)
                return;
            // 先从已有的父对象中移除
            if (children->m_Parent)
            {
                children->m_Parent->RemoveChildren(children);
            }
            children->m_Parent = this;
            m_Children.insert(children);
        }

        virtual ~Object()
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
            // 从父对象列表中移除
            if (const auto parent = GetParent())
            {
                parent->RemoveChildren(this);
                m_Parent = nullptr;
            }
            // 递归删除子对象
            while (!m_Children.empty())
            {
                const auto iter = m_Children.begin();
                Object* child = *iter;

                // 先解除父子关系，再销毁子对象。这样子对象析构时
                // 不会回头修改当前正在遍历的 m_Children。
                m_Children.erase(iter);
                child->m_Parent = nullptr;
                delete child;
            }
            std::cout << "[ " << m_Name << " ]" << " Destroyed End!" << std::endl;
        }

    private:
        bool RemoveChildren(Object* children)
        {
            if (const auto iter = m_Children.find(children); iter != m_Children.end())
            {
                m_Children.erase(iter);
                return true;
            }
            return false;
        }

    private:
        std::unordered_set<Object*> m_Children;
        std::string m_Name;
        Object* m_Parent = nullptr;
    };
}

namespace Version3
{
    class Object
    {
    public:
        explicit Object(std::string name, Object* parent = nullptr): m_Name(std::move(name))
        {
            SetParent(parent);
        }

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        [[nodiscard]] Object* GetParent() const
        {
            return m_Parent;
        }

        void SetParent(Object* newParent)
        {
            if (m_Parent == newParent)
                return;
            for (auto ancestor = newParent; ancestor; ancestor = ancestor->GetParent())
            {
                // 如果newParent的父对象中有自己，就会形成环，这样情况应该被禁止
                if (ancestor == this)
                {
                    throw std::logic_error("An object cannot become a child of itself or its descendant");
                }
            }
            Object* oldParent = m_Parent;
            // 加入新父对象
            if (newParent)
                newParent->AttachChild(this);

            // 从旧父对象移除
            if (oldParent)
                oldParent->DetachChild(this);
            m_Parent = newParent;
        }

        [[nodiscard]] std::size_t ChildCount() const noexcept
        {
            return m_Children.size();
        }

        [[nodiscard]] bool HasChild(Object* child) const noexcept
        {
            return m_Children.contains(child);
        }

        virtual ~Object()
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
            // 从父对象列表中移除
            SetParent(nullptr);
            // 递归删除子对象
            while (!m_Children.empty())
            {
                const auto iter = m_Children.begin();
                Object* child = *iter;
                // 先解除父子关系，再销毁子对象。这样子对象析构时
                // 不会回头修改当前正在遍历的 m_Children。
                m_Children.erase(iter);
                child->m_Parent = nullptr;
                delete child;
            }
            std::cout << "[ " << m_Name << " ]" << " Destroyed End!" << std::endl;
        }

    private:
        void DetachChild(Object* children)
        {
            m_Children.erase(children);
        }

        void AttachChild(Object* children)
        {
            m_Children.insert(children);
        }

    private:
        std::unordered_set<Object*> m_Children;
        std::string m_Name;
        Object* m_Parent = nullptr;
    };
}

namespace Version4
{
    class Object
    {
    public:
        explicit Object(std::string name): m_Name(std::move(name))
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

        virtual ~Object()
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
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

    private:
        std::unordered_map<Object*, std::unique_ptr<Object>> m_Children;
        std::string m_Name;
        Object* m_Parent = nullptr;
    };
}

namespace Version5
{
    class Object;

    template <class T>
    concept DerivedFrom = std::derived_from<T, Object>;

    class LifetimeToken
    {
    };

    template <DerivedFrom T>
    class ObjectPtr;

    class Object
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

        virtual ~Object()
        {
            std::cout << "[ " << m_Name << " ]" << " Destroyed Start!" << std::endl;
            m_LifetimeToken.reset();
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

    private:
        std::unordered_map<Object*, std::unique_ptr<Object>> m_Children;
        std::string m_Name;
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
}


class ObjectTreeManager
{
public:
    ObjectTreeManager()
    {
        test5();
    }

    void test1()
    {
        using namespace Version1;
        auto o1 = new Object("o1");
        auto o2 = new Object("o2", o1);
        auto o3 = new Object("o3", o1);
        auto o4 = new Object("o4", o2);
        delete o2;
        delete o1;
        delete o3;
        delete o4;
    }

    void test2()
    {
        using namespace Version2;
        auto o1 = new Object("o1");
        auto o2 = new Object("o2", o1);
        auto o3 = new Object("o3");
        auto o4 = new Object("o4");
        auto o5 = new Object("o5");
        o3->SetParent(o1);
        o2->AddChildren(o4);
        o5->AddChildren(o2);
        o5->SetParent(o1);
        delete o2;
        delete o1;
        delete o3;
        delete o4;
        delete o5;
    }

    void test3()
    {
        using namespace Version3;
        auto o1 = new Object("o1");
        auto o2 = new Object("o2", o1);
        auto o3 = new Object("o3");

        assert(o1->HasChild(o2));
        // 真正的换父
        o3->SetParent(o2);
        assert(o2->HasChild(o3));

        // 脱离对象树
        o3->SetParent(nullptr);

        // 重复设置相同父对象
        o3->SetParent(o2);
        o3->SetParent(o2);

        // 自己成为自己的父对象
        bool rejected = false;
        try
        {
            o3->SetParent(o3);
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }
        assert(rejected);
        rejected = false;
        // 祖先变成后代的子对象
        try
        {
            o2->SetParent(o1);
            o1->SetParent(o2);
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }
        assert(rejected);

        // 子对象提前销毁
        delete o3;
        delete o1;
        delete o2;
    }

    void test4()
    {
        using namespace Version4;
        auto root = std::make_unique<Object>("root");
        auto o1 = std::make_unique<Object>("o1");
        auto o2 = std::make_unique<Object>("o2");
        auto o3 = std::make_unique<Object>("o3");
        auto o4 = std::make_unique<Object>("o4");
        auto o5 = std::make_unique<Object>("o5");
        auto rawo1 = o1.get();
        auto rawo2 = o2.get();
        auto rawo3 = o3.get();
        auto rawo4 = o4.get();
        auto rawo5 = o5.get();
        root->AttachChild(std::move(o1));
        rawo1->AttachChild(std::move(o2));
        root->AttachChild(std::move(o3));
        rawo1->AttachChild(std::move(o4));
        rawo4->AttachChild(std::move(o5));

        assert(rawo1->HasChild(rawo2));
        assert(rawo1->HasChild(rawo4));
        assert(root->HasChild(rawo3));

        rawo4->Reparent(rawo3);
        assert(rawo3->HasChild(rawo4));
    }

    void test5()
    {
        using namespace Version5;
        auto root = std::make_unique<Object>("root");
        auto o1 = std::make_unique<Object>("o1");
        auto o2 = std::make_unique<Object>("o2");
        auto o3 = std::make_unique<Object>("o3");
        auto o4 = std::make_unique<Object>("o4");
        auto o5 = std::make_unique<Object>("o5");
        ObjectPtr rawo1(o1.get());
        ObjectPtr rawo2(o2.get());
        ObjectPtr rawo3(o3.get());
        ObjectPtr rawo4(o4.get());
        ObjectPtr rawo5(o5.get());

        root->AttachChild(std::move(o1));
        rawo1->AttachChild(std::move(o2));
        root->AttachChild(std::move(o3));
        rawo1->AttachChild(std::move(o4));
        rawo4->AttachChild(std::move(o5));

        assert(rawo1->HasChild(rawo2.Get()));
        assert(rawo1->HasChild(rawo4.Get()));
        assert(root->HasChild(rawo3.Get()));

        rawo4->Reparent(rawo3.Get());
        assert(rawo3->HasChild(rawo4.Get()));

        // 复制观察者测试
        ObjectPtr copy1 = rawo3;
        ObjectPtr copy2 = copy1;
        assert(copy1);
        assert(copy2);

        // 移动观察者测试
        ObjectPtr move1 = std::move(rawo1);
        assert(move1);
        assert(rawo1);

        // 手动Reset
        ObjectPtr observer = rawo2;
        observer.Reset();
        assert(!observer);
        assert(rawo2); // 重置一个观察者不影响其他观察者

        auto o3Owner = root->DetachChild(rawo3.Get());
        o3Owner.reset();
        assert(!rawo3);
        assert(!rawo4);
        assert(!rawo5);
        assert(copy1);
        assert(copy2);
    }
};

static ObjectTreeManager signal_connect_manager;




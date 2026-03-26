//
// Created by ZQD on 26-3-24.
//

#pragma once
#include <iostream>
#include <map>
#include <unordered_map>

namespace MyTest
{
    class Object
    {
    public:
        explicit Object(int id) : m_Id(id)
        {
        }

    public:
        void DoSomething()
        {
            std::cout << "Hello,ObjectId =" << m_Id << std::endl;
        }

    public:
        int m_Id;
    };

    class ElmHandle
    {
    public:
        ElmHandle()
        {
            m_Object = nullptr;
            count = 0;
        }

        explicit ElmHandle(Object* obj) : m_Object(obj)
        {
            Inc();
        }

        ElmHandle(const ElmHandle&& elm) noexcept
        {
            m_Object = elm.m_Object;
            count = elm.count;
            isRegister = elm.isRegister;
        }

        ElmHandle(const ElmHandle& elm) noexcept
        {
            m_Object = elm.m_Object;
            count = elm.count;
            isRegister = elm.isRegister;
        }

        ~ElmHandle()
        {
            Dec();
            if (IsExpired())
            {
                delete m_Object;
                m_Object = nullptr;
            }
        }

        void Inc()
        {
            ++count;
        }

        void Dec()
        {
            --count;
        }

        bool IsExpired() const
        {
            return count == 0 || !isRegister;
        }

        void DoSomething() const
        {
            if (!IsExpired())
            {
                m_Object->DoSomething();
            }
        }

    public:
        Object* m_Object = nullptr;
        int count = 0;
        mutable bool isRegister = false;
    };

    class MiniDocument
    {
    public:
        ElmHandle* CreateObject(int id)
        {
            const auto obj = new Object(id);
            auto handle = new ElmHandle(obj);
            handle->isRegister = true;
            m_Doc.insert(std::pair<int, ElmHandle*>(id, handle));
            return handle;
        }

        void Delete(int id)
        {
            if (m_Doc.find(id) != m_Doc.end())
            {
                const auto handle = m_Doc[id];
                handle->isRegister = false;
                m_Doc.erase(id);
            }
        }

    private:
        std::map<int, ElmHandle*> m_Doc;
    };

    class Test
    {
    public:
        Test()
        {
            MiniDocument doc;
            auto h = doc.CreateObject(1);
            h->DoSomething();
            {
                doc.Delete(1);
            }
            if (!h->IsExpired())
            {
                h->DoSomething();
            }
            delete h;
        }
    };

    static Test t;
};

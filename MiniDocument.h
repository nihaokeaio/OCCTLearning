//
// Created by ZQD on 26-3-24.
//

#pragma once
#include <assert.h>
#include <iostream>
#include <map>
#include <unordered_map>

namespace MyTest
{
    class ControlBlock;

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
        ControlBlock* m_ControlBlock = nullptr;
        int m_Id;
    };

    class ControlBlock
    {
    public:
        Object* m_Object;
        int m_RefCount;
        bool m_IsRegister;
    };


    class ElmHandle
    {
    public:
        ElmHandle() = delete;

        explicit ElmHandle(ControlBlock* controlBlock) : m_ControlBlock(controlBlock)
        {
            assert(controlBlock==nullptr);
            Inc();
        }

        ElmHandle(const ElmHandle&& elm) noexcept
        {
            m_ControlBlock = elm.m_ControlBlock;
            Inc();
        }

        ElmHandle(const ElmHandle& elm) noexcept
        {
            m_ControlBlock = elm.m_ControlBlock;
            Inc();
        }

        ~ElmHandle()
        {
            Dec();
            if (m_ControlBlock->m_RefCount == 0)
            {
                delete m_ControlBlock->m_Object;
                delete m_ControlBlock;
            }
        }

        void Inc()
        {
            ++m_ControlBlock->m_RefCount;
        }

        void Dec()
        {
            --m_ControlBlock->m_RefCount;
        }

        bool IsExpired() const
        {
            return !m_ControlBlock->m_IsRegister;
        }

        void DoSomething() const
        {
            if (!IsExpired())
            {
                m_ControlBlock->m_Object->DoSomething();
            }
        }

    public:
        ControlBlock* m_ControlBlock = nullptr;
    };

    class MiniDocument
    {
    public:
        ElmHandle* CreateObject(int id)
        {
            const auto obj = new Object(id);
            auto controlBlock = new ControlBlock();
            obj->m_ControlBlock = controlBlock;
            controlBlock->m_Object = obj;
            controlBlock->m_IsRegister = true;
            const auto handle = new ElmHandle(controlBlock);
            m_Doc.insert(std::pair<int, Object*>(id, obj));
            return handle;
        }

        void Delete(int id)
        {
            if (m_Doc.find(id) != m_Doc.end())
            {
                const auto object = m_Doc[id];
                object->m_ControlBlock->m_IsRegister = false;
                m_Doc.erase(id);
            }
        }

    private:
        std::map<int, Object*> m_Doc;
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

//
// Created by ZQD on 26-8-7.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "ElementId.h"


namespace MiniMetaObject {
    class RegisterObject;
}

class Element;

class Document
{
public:
    Document();

    void RegisterElement(std::unique_ptr<Element>&& element);

    std::unique_ptr<Element> UnregisterElement(const ElementId& elementId);

    static ElementId NewElementId();

    Element* FindElement(const ElementId& elementId);

    template <typename T>
    T* FindElement(const ElementId& elementId);

    MiniMetaObject::RegisterObject *GetMetaRegister() const;

private:
    std::unordered_map<ElementId, std::unique_ptr<Element>> m_Elements;
    std::unique_ptr<MiniMetaObject::RegisterObject> m_MetaRegister;
};

template <typename T>
T* Document::FindElement(const ElementId& elementId)
{
    if (const auto iter = m_Elements.find(elementId); iter != m_Elements.end())
    {
        return dynamic_cast<T*>(iter->second.get());
    }
    return nullptr;
}



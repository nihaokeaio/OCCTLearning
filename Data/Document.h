//
// Created by ZQD on 26-8-7.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "ElementId.h"


class Element;

class Document
{
public:
    void RegisterElement(std::unique_ptr<Element>&& element);

    std::unique_ptr<Element> UnregisterElement(const ElementId& elementId);

    static ElementId NewElementId();

    Element* FindElement(const ElementId& elementId);

    template <typename T>
    T* FindElement(const ElementId& elementId);

private:
    std::unordered_map<ElementId, std::unique_ptr<Element>> m_Elements;
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



//
// Created by ZQD on 26-8-7.
//

#include "Document.h"

#include "Document.h"

#include "Element.h"
#include "GlobalUniqueId.h"
#include "MessageInfo.h"

void Document::RegisterElement(std::unique_ptr<Element>&& element)
{
    if (element->GetId() == ElementId::InvalidId)
    {
        element->SetId(NewElementId());
    }
    auto rawElement = element.get();
    element->SetDocument(this);
    m_Elements.insert(std::make_pair(element->GetId(), std::move(element)));
    rawElement->NotifyElementChanged(MessageInfo::ElementChangeFlag::Register);
}

std::unique_ptr<Element> Document::UnregisterElement(const ElementId& elementId)
{
    if (const auto it = m_Elements.find(elementId); it != m_Elements.end())
    {
        it->second->NotifyElementChanged(MessageInfo::ElementChangeFlag::Unregister);
        auto element = std::move(it->second);
        m_Elements.erase(it);
        element->SetDocument(nullptr);
        return element;
    }
    return nullptr;
}

ElementId Document::NewElementId()
{
    return ElementId(GlobalUniqueId::Instance().NextId());
}

Element* Document::FindElement(const ElementId& elementId)
{
    if (const auto iter = m_Elements.find(elementId); iter != m_Elements.end())
    {
        return iter->second.get();
    }
    return nullptr;
}

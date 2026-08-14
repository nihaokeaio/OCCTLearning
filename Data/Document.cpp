//
// Created by ZQD on 26-8-7.
//

#include "Document.h"
#include "Element.h"
#include "ElementDerived.h"
#include "GlobalUniqueId.h"
#include "MessageInfo.h"

Document::Document() {
    m_MetaRegister = std::make_unique<MiniMetaObject::RegisterObject>();
    m_MetaRegister->Register<Element>();
    m_MetaRegister->Register<PointElement>();
    m_MetaRegister->Register<SegmentElement>();
    m_MetaRegister->Register<CircleElement>();
    m_MetaRegister->Register<MetricsElement>();
}

Document::~Document() = default;

void Document::RegisterElement(std::unique_ptr<Element>&& element)
{
    if (element->GetId() == ElementId::InvalidId)
    {
        element->SetId(NewElementId());
    }
    auto rawElement = element.get();
    element->SetDocument(this);
    m_Elements.insert(std::make_pair(element->GetId(), std::move(element)));
    NotifyElementChanged(rawElement->GetId(), MessageInfo::ElementChangeFlag::Register);
}

std::unique_ptr<Element> Document::UnregisterElement(const ElementId& elementId)
{
    if (const auto it = m_Elements.find(elementId); it != m_Elements.end())
    {
        NotifyElementChanged(elementId, MessageInfo::ElementChangeFlag::Unregister);
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

MiniMetaObject::RegisterObject *Document::GetMetaRegister() const {
    return m_MetaRegister.get();
}

void Document::NotifyElementChanged(const ElementId elementId, MessageInfo::ElementChangeFlag flag)
{
    m_ElementChangedSignal.emit({flag, elementId});
}

void Document::NotifyElementPropertyChanged(const MessageInfo::PropertyChangePayload& message)
{
    m_ElementPropertyChangedSignal.emit(message);
}

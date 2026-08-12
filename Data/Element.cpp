//
// Created by ZQD on 26-8-7.
//

#include "Element.h"

#include <cassert>

#include "SignalConnectManager.h"

Element::Element() : m_Id(ElementId::InvalidId) {
    m_Name = "Element";
    NotifyElementChanged(MessageInfo::ElementChangeFlag::Create);
}

const MiniMetaObject::MetaObject* Element::GetStaticMetaObject() noexcept
{
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{
        "Element", MiniMetaObject::Object::GetStaticMetaObject(),
        {
            MakeMemberProperty("M_Properties", &Element::m_Properties)
        },
        {
            MakeMetaMethod("F_GetProperties", &Element::GetProperty),
            MakeMetaMethod("F_SetProperties", &Element::SetProperty)
        }
    };
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject* Element::GetMetaObject() const noexcept
{
    return GetStaticMetaObject();
}

Element::~Element()
{
    NotifyElementChanged(MessageInfo::ElementChangeFlag::Remove);
}

Document* Element::GetDocument() const
{
    return m_Document;
}

void Element::SetDocument(Document* doc)
{
    m_Document = doc;
}

bool Element::HasProperty(const std::string_view key) const
{
    return m_Properties.Exists(std::string(key));
}

void Element::NotifyElementChanged(MessageInfo::ElementChangeFlag flag)
{
    assert(m_Document != nullptr);
    const auto message = std::make_shared<MessageInfo::ElementChangePayload>(m_Id);
    m_ElementChangeSignal.emit(flag, message);
}


ElementId Element::GetId() const
{
    return m_Id;
}

void Element::SetId(const ElementId& elementId)
{
    m_Id = elementId;
}

std::string Element::GetName()
{
    return m_Name;
}


PropertySet& Element::Properties()
{
    return m_Properties;
}

const PropertySet& Element::Properties() const
{
    return m_Properties;
}

void Element::SetProperty(const std::string& key, const PropertyValue& value)
{
    const auto message = std::make_shared<MessageInfo::ElementPropertyChangePayload>(m_Id, key, value);
    m_ElementChangeSignal.emit(MessageInfo::ElementChangeFlag::Update, message);
    m_Properties.Set(key, value);
}

std::optional<PropertyValue> Element::GetProperty(const std::string &key) const {
    if (m_Properties.Exists(key)) {
        return m_Properties.Get(key);
    }
    return std::nullopt;
}


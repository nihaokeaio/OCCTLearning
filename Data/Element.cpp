//
// Created by ZQD on 26-8-7.
//

#include "Element.h"

#include "Document.h"
#include "SignalConnectManager.h"

Element::Element() : m_Id(ElementId::InvalidId) {
    m_Name = "Element";
}

const MiniMetaObject::MetaObject* Element::GetStaticMetaObject() noexcept
{
    using namespace MiniMetaObject;
    static const MetaObject StaticMetaObject{"Element", MiniMetaObject::Object::GetStaticMetaObject()};
    return &StaticMetaObject;
}

const MiniMetaObject::MetaObject* Element::GetMetaObject() const noexcept
{
    return GetStaticMetaObject();
}

Element::~Element() = default;

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

const PropertySet& Element::Properties() const
{
    return m_Properties;
}

void Element::NotifyPropertyChanged(const PropertyAddress& address, const PropertyValue& oldValue,
                                    const PropertyValue& newValue, const ChangeSource source) const
{
    m_Document->NotifyElementPropertyChanged(MessageInfo::PropertyChangePayload{
        {GetId(), address.propertyName},
        oldValue,
        newValue,
        source
    });
}

bool Element::SetProperty(const std::string& key, const PropertyValue& value, ChangeSource source)
{
    const auto oldValue = m_Properties.Get(key);
    if (!oldValue)
        return false;
    m_Properties.Set(key, value);
    NotifyPropertyChanged({GetId(), key}, oldValue, value, source);
    return true;
}

bool Element::SetPropertyDirectly(const std::string& key, const PropertyValue& value)
{
    // 直接写入不再做检查
    m_Properties.Set(key, value);
    return true;
}

std::optional<PropertyValue> Element::GetProperty(const std::string &key) const {
    if (m_Properties.Exists(key)) {
        return m_Properties.Get(key);
    }
    return std::nullopt;
}


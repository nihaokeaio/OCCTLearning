//
// Created by ZQD on 26-8-12.
//

#include "PropertyResolver.h"

#include "Data/Document.h"
#include "Data/Element.h"

PropertyResolver::PropertyResolver(Document* document): m_Document(document)
{
}

std::optional<PropertyValue> PropertyResolver::Read(const PropertyAddress& address) const
{
    if (const auto metaProperty = Resolve(address))
    {
        const auto element = m_Document->FindElement(address.elementId);
        assert(element);
        return metaProperty->Read(element);
    }
    return std::nullopt;
}

bool PropertyResolver::Write(const PropertyAddress& address, const PropertyValue& value, ChangeSource source) const
{
    const auto element = m_Document->FindElement(address.elementId);
    if (!element)
        return false;
    const auto metaProperty = Resolve(address);
    if (!metaProperty || !metaProperty->IsWritable())
        return false;

    if (metaProperty->Type() != value.Type())
        return false;

    const PropertyValue oldValue = metaProperty->Read(element);

    if (!metaProperty->Write(element, value))
        return false;
    // 写入后重新读取
    const PropertyValue newValue = metaProperty->Read(element);
    // 待需要时添加比较操作
    element->NotifyPropertyChanged(address, oldValue, newValue, source);
    return true;
}

const MiniMetaObject::MetaProperty* PropertyResolver::Resolve(const PropertyAddress& address) const
{
    if (const auto element = m_Document->FindElement(address.elementId))
    {
        if (const auto metaObject = element->GetMetaObject())
        {
            return metaObject->FindProperty(address.propertyName);
        }
    }
    return nullptr;
}

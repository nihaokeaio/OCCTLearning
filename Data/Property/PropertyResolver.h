//
// Created by ZQD on 26-8-12.
//

#pragma once
#include "PropertyAddress.h"
#include "PropertySet.h"


class Document;

class PropertyResolver
{
public:
    explicit PropertyResolver(Document* document);

    std::optional<PropertyValue> Read(const PropertyAddress& address) const;

    bool Write(const PropertyAddress& address, const PropertyValue& value, ChangeSource source) const;

private:
    const MiniMetaObject::MetaProperty* Resolve(const PropertyAddress& address) const;

private:
    Document* m_Document;
};

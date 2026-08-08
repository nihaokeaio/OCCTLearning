//
// Created by ZQD on 26-8-7.
//

#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>

#include "ElementId.h"
#include "MessageInfo.h"
#include "MetaObjectManager.h"
#include "ObjectTreeManager.h"
#include "SignalConnectManager.h"
#include "Property/PropertySet.h"

class Document;


class Element : public MiniMetaObject::Object
{
public:
    Element();
    static const MiniMetaObject::MetaObject* GetStaticMetaObject() noexcept;
    [[nodiscard]] const MiniMetaObject::MetaObject* GetMetaObject() const noexcept override;


    [[nodiscard]] Document* GetDocument() const;

    void SetDocument(Document* doc);


    [[nodiscard]] ElementId GetId() const;

    void SetId(const ElementId& elementId);

    ~Element() override;

    std::string GetName();

    [[nodiscard]] PropertySet& Properties();

    [[nodiscard]] const PropertySet& Properties() const;

    void SetProperty(const std::string& key, const PropertyValue& value);

    std::optional<PropertyValue> GetProperty(const std::string &key) const;

    [[nodiscard]] bool HasProperty(std::string_view key) const;

    void NotifyElementChanged(MessageInfo::ElementChangeFlag);

protected:
    std::string m_Name;
    ElementId m_Id;
    Document* m_Document;
    PropertySet m_Properties;
    MiniSignal::Signal<MessageInfo::ElementChangeFlag, std::shared_ptr<MessageInfo::MessagePayload>>
    m_ElementChangeSignal;
};


#endif //ELEMENT_H

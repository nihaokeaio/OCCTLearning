//
// Created by ZQD on 26-8-7.
//

#pragma once
#include "ElementId.h"
#include "Property/PropertySet.h"

namespace MessageInfo
{
    enum class ElementChangeFlag
    {
        Create,
        Remove,
        Register,
        Unregister,
        Update
    };

    struct MessagePayload
    {
        virtual ~MessagePayload() = default;
    };

    struct ElementChangePayload : MessagePayload
    {
        explicit ElementChangePayload(const ElementId elementId) : id(std::move(elementId))
        {
        }

        ElementId id;
    };

    struct ElementPropertyChangePayload : MessagePayload {
        explicit ElementPropertyChangePayload(const ElementId elementId, const std::string_view propertyKey,
                                              PropertyValue newV) : id(std::move(elementId)),
                                                                    key(propertyKey),
                                                                    newValue(std::move(newV)) {
        }

        ElementId id;
        std::string_view key;
        PropertyValue newValue;
    };
}

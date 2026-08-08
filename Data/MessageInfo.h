//
// Created by ZQD on 26-8-7.
//

#pragma once
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
    };

    struct ElementChangePayload : MessagePayload
    {
        explicit ElementChangePayload(const uint64_t elementId) : id(elementId) {
        }

        uint64_t id;
    };

    struct ElementPropertyChangePayload : MessagePayload {
        explicit ElementPropertyChangePayload(const uint64_t elementId, const std::string_view propertyKey,
                                              PropertyValue newV) : id(elementId),
                                                                    key(propertyKey),
                                                                    newValue(std::move(newV)) {
        }

        uint64_t id;
        std::string_view key;
        PropertyValue newValue;
    };
}

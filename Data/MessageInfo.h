//
// Created by ZQD on 26-8-7.
//

#pragma once
#include "ElementId.h"
#include "Property/PropertyAddress.h"
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

    struct PropertyChangePayload
    {
        PropertyAddress address;
        PropertyValue oldValue;
        PropertyValue newValue;
        ChangeSource source;
    };

    struct ElementChangePayload
    {
        ElementChangeFlag flag;
        ElementId id;
    };

}

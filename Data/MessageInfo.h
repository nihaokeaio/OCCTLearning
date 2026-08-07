//
// Created by ZQD on 26-8-7.
//

#pragma once

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
        uint64_t id;
    };
}

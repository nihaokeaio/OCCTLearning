#pragma once

#include "DependencyGraphIds.h"

#include <functional>

struct Render
{
    using UpdateCallback = std::function<void(ValueId)>;

    void SetUpdateCallback(UpdateCallback callback);
    void Update(ValueId node);

private:
    UpdateCallback m_UpdateCallback;
};

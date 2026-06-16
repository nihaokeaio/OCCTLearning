#include "Render.h"

#include <utility>

void Render::SetUpdateCallback(UpdateCallback callback)
{
    m_UpdateCallback = std::move(callback);
}

void Render::Update(ValueId node)
{
    if (m_UpdateCallback)
    {
        m_UpdateCallback(node);
    }
}

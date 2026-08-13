//
// Created by ZQD on 26-8-12.
//

#include "DependencyBinding.h"
#include "DGContext.h"
#include "Data/Property/PropertyAddress.h"

DependencyBinding::DependencyBinding(DGContext* context): m_DGContext(context)
{
}

void DependencyBinding::OnPropertyChanged(const PropertyChange& change) const
{
    if (change.changeSource == ChangeSource::DependencyGraph)
        return;

    if (!m_DGContext->HasValue(change.address))
        return;

    m_DGContext->MarkDirty(change.address);
}

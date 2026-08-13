//
// Created by ZQD on 26-8-12.
//

#pragma once

struct PropertyChange;
struct DGContext;

class DependencyBinding
{
public:
    explicit DependencyBinding(DGContext* context);

    void OnPropertyChanged(const PropertyChange& change) const;

private:
    DGContext* m_DGContext;
};



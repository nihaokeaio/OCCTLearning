#pragma once

#include "../DependencyGraph/DGContext.h"

#include <gp_Pnt.hxx>

#include <string>

#include "Data/Document.h"


class SketchModel
{
public:
    explicit SketchModel(DGContext *context, Document *document);

    void BuildDemoGraph();

    Element *CreateElement(std::string_view elementName) const;

    void AddPropertyAddress(PropertyAddress address) const;

    void AddSegmentComputerNode(std::vector<PropertyAddress> in, std::vector<PropertyAddress> out) const;

    void AddCircleAreaComputerNode(std::vector<PropertyAddress> in, std::vector<PropertyAddress> out) const;

    void AddComputerNode(std::vector<PropertyAddress> in, std::vector<PropertyAddress> out,
                         ComputerNode::ComputeFunc computeFunc) const;

private:
    DGContext *m_Context;
    Document *m_Document;
};

#pragma once

#include "DGContext.h"
#include "../Demo/SketchModel.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_TextLabel.hxx>
#include <gp_Pnt.hxx>
#include <V3d_View.hxx>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class DependencyGraphManager
{
public:
    DependencyGraphManager();

    ~DependencyGraphManager();

    void InitializeDemoScene(const Handle(AIS_InteractiveContext)& context);

    void EvaluateAndRefreshScene() const;

private:
    void BuildDemoGraph();

private:
    std::unique_ptr<DGContext> m_Context;
    std::unique_ptr<SketchModel> m_Sketch;
    std::unique_ptr<Document> m_Document;
    Handle(AIS_InteractiveContext) m_AisContext;
    std::vector<PropertyAddress> m_Points;
    std::vector<PropertyAddress> m_Segments;
    std::vector<PropertyAddress> m_Circles;
};

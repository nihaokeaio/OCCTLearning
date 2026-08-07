#pragma once

#include "DependencyGraph/Binding/ValueBindingRegistry.h"
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
    void Test();
    void InitializeDemoScene(const Handle(AIS_InteractiveContext)& context);
    void RefreshScene();
    void RenderGuiControls();
    bool BeginPointDrag(const Handle(V3d_View)& view, int screenX, int screenY);
    bool DragPointTo(const Handle(V3d_View)& view, int screenX, int screenY);
    void EndPointDrag();
    [[nodiscard]] bool IsDraggingPoint() const;

private:
    struct RenderedPoint
    {
        std::string name;
        SketchPoint point;
        Handle(AIS_Shape) shape;
    };

    struct RenderedSegment
    {
        SketchSegment segment;
        Handle(AIS_Shape) shape;
    };

    struct RenderedDistanceDimension
    {
        SketchDistanceDimension dimension;
        Handle(AIS_TextLabel) label;
    };

    void BuildDemoGraph();
    void DisplayScene();
    void EvaluateAndRefreshScene();
    void MovePoint(size_t index, const gp_Pnt& position);
    bool TryPickPointAtScreen(const Handle(V3d_View)& view, int screenX, int screenY, size_t& pointIndex) const;
    bool ProjectScreenToSketchPlane(const Handle(V3d_View)& view, int screenX, int screenY, gp_Pnt& point) const;
    void RefreshChangedScene();
    void RefreshPoint(RenderedPoint& renderedPoint);
    void RefreshSegment(RenderedSegment& renderedSegment);
    void RefreshDistanceDimension(RenderedDistanceDimension& renderedDimension);
    void RegisterDemoBinding(ValueId valueId, std::string objectId, std::string propertyKey);

private:
    DGContext m_Context;
    ValueBindingRegistry m_Bindings;
    std::unique_ptr<SketchModel> m_Sketch;
    Handle(AIS_InteractiveContext) m_AisContext;
    std::unordered_set<ValueId> m_ChangedValues;
    size_t m_DraggingPointIndex = static_cast<size_t>(-1);
    bool m_TraceEnabledBeforeDrag = true;
    std::vector<std::string> m_FlowLogLines;
    std::vector<RenderedPoint> m_Points;
    std::vector<RenderedSegment> m_Segments;
    std::vector<RenderedDistanceDimension> m_DistanceDimensions;
};

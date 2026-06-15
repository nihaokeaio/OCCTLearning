#include "DependencyGraphManager.h"

#include <gp_Pnt.hxx>

#include "Sketch/SketchModel.h"

void DependencyGraphManager::Test()
{
    SketchModel sketch(m_Context);

    const auto j0 = sketch.CreatePoint(gp_Pnt(0, 0, 0), "j0");
    const auto j1 = sketch.CreatePoint(gp_Pnt(100, 0, 0), "j1");
    const auto j2 = sketch.CreatePoint(gp_Pnt(100, 100, 0), "j2");

    const auto s0 = sketch.CreateSegment(j0, j1, "s0");
    const auto s1 = sketch.CreateSegment(j1, j2, "s1");

    sketch.CreateRectangleArea(s0, s1, "rect");
    sketch.CreateCircleAreaFromRadius(s0, "circle");
    sketch.AddLengthRenderNode(s0, "s0");
    sketch.AddLengthRenderNode(s1, "s1");

    sketch.MovePoint(j0, gp_Pnt{10, 0, 0});
    sketch.MovePoint(j2, gp_Pnt{100, 200, 0});
    sketch.Evaluate();
}

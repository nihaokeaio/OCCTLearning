#include "DependencyGraphManager.h"

#include "imgui/imgui.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopoDS_Shape.hxx>

#include <cmath>
#include <iomanip>
#include <sstream>

#include "Data/Element.h"
#include "Data/Document.h"
#include "Demo/SketchRuntime.h"

namespace
{
    constexpr double PointRadius = 4.0;
    constexpr double DimensionTextOffset = 12.0;
    constexpr double PointPickRadiusPx = 14.0;
    constexpr double SketchPlaneZ = 0.0;

    TopoDS_Shape MakePointShape(const gp_Pnt& point)
    {
        return BRepPrimAPI_MakeSphere(point, PointRadius).Shape();
    }

    TopoDS_Shape MakeSegmentShape(const gp_Pnt& start, const gp_Pnt& end)
    {
        return BRepBuilderAPI_MakeEdge(start, end).Shape();
    }

    gp_Pnt MidPoint(const gp_Pnt& start, const gp_Pnt& end)
    {
        return gp_Pnt{
            (start.X() + end.X()) * 0.5,
            (start.Y() + end.Y()) * 0.5,
            (start.Z() + end.Z()) * 0.5
        };
    }

    std::string FormatDistance(double distance)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << distance;
        return out.str();
    }

    constexpr auto PositionProperty = "Position";
    constexpr auto LengthProperty = "Length";
    constexpr auto AreaProperty = "Area";
}

DependencyGraphManager::DependencyGraphManager() {
    m_SketchRuntime = std::make_unique<SketchRuntime>();
}

DependencyGraphManager::~DependencyGraphManager() = default;

void DependencyGraphManager::InitializeDemoScene(const Handle(AIS_InteractiveContext)& context)
{
    m_AisContext = context;
    BuildDemoGraph();
}

void DependencyGraphManager::BuildDemoGraph()
{
    const auto sketch = m_SketchRuntime->GetSketchModel();
    m_Points.clear();
    m_Segments.clear();

    auto p0 = sketch->CreateElement("PointElement");
    auto p1 = sketch->CreateElement("PointElement");
    auto s0 = sketch->CreateElement("SegmentElement");
    auto c0 = sketch->CreateElement("CircleElement");
    auto lambdaFun = [this](const MessageInfo::PropertyChangePayload& message)
    {
        printf("Element [%s] Property change! [key]= %s \n", message.address.elementId.ToString().c_str(),
               message.address.propertyName.c_str());
    };
    MiniSignal::connect(m_SketchRuntime->GetDocument(), &Document::m_ElementPropertyChangedSignal, lambdaFun);
    auto p0A = PropertyAddress{p0->GetId(), PositionProperty};
    auto p1A = PropertyAddress{p1->GetId(), PositionProperty};
    auto s0A = PropertyAddress{s0->GetId(), LengthProperty};
    auto c0A = PropertyAddress{c0->GetId(), AreaProperty};
    sketch->AddPropertyAddress(p0A);
    sketch->AddPropertyAddress(p1A);
    sketch->AddPropertyAddress(s0A);
    sketch->AddPropertyAddress(c0A);

    m_Points.push_back(p0A);
    m_Points.push_back(p1A);
    m_Segments.push_back(s0A);
    m_Circles.push_back(c0A);

    m_SketchRuntime->SetProperty(p0A, gp_Pnt(0, 0, 0), ChangeSource::User);
    m_SketchRuntime->SetProperty(p1A, gp_Pnt(100, 0, 0), ChangeSource::User);

    sketch->AddSegmentComputerNode({p0A, p1A}, {s0A});
    sketch->AddCircleAreaComputerNode({s0A}, {c0A});
}

void DependencyGraphManager::EvaluateAndRefreshScene() const
{
    const auto result = m_SketchRuntime->Flush();
}


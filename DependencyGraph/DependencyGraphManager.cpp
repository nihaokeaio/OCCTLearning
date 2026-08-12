#include "DependencyGraphManager.h"

#include "imgui/imgui.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Quantity_Color.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopoDS_Shape.hxx>

#include <cmath>
#include <iomanip>
#include <sstream>

#include "Data/Element.h"
#include "Data/Document.h"

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
    m_Document = std::make_unique<Document>();
    m_Context = std::make_unique<DGContext>(m_Document.get());
}

DependencyGraphManager::~DependencyGraphManager() = default;

void DependencyGraphManager::InitializeDemoScene(const Handle(AIS_InteractiveContext)& context)
{
    m_AisContext = context;
    BuildDemoGraph();
}

void DependencyGraphManager::BuildDemoGraph() {
    m_Context->Clear();
    m_Sketch = std::make_unique<SketchModel>(m_Context.get(), m_Document.get());
    m_Points.clear();
    m_Segments.clear();

    auto p0 = m_Sketch->CreateElement("PointElement");
    auto p1 = m_Sketch->CreateElement("PointElement");
    auto s0 = m_Sketch->CreateElement("SegmentElement");
    auto c0 = m_Sketch->CreateElement("CircleElement");
    auto lambdaFun = [this](MessageInfo::ElementChangeFlag flag,
                            const std::shared_ptr<MessageInfo::MessagePayload>& message)
    {
        if (auto pECPayload = dynamic_cast<MessageInfo::ElementChangePayload*>(message.get()))
        {
            printf("Element [%s] change! \n", pECPayload->id.ToString().c_str());
        }
        if (auto pEPCPayload = dynamic_cast<MessageInfo::ElementPropertyChangePayload*>(message.
            get()))
        {
            printf("Element [%s] Property change! [key]= %s \n", pEPCPayload->id.ToString().c_str(),
                   pEPCPayload->key.data());
        }
    };
    MiniSignal::connect(p0, &Element::m_ElementChangeSignal, lambdaFun);
    MiniSignal::connect(p1, &Element::m_ElementChangeSignal, lambdaFun);
    MiniSignal::connect(s0, &Element::m_ElementChangeSignal, lambdaFun);
    MiniSignal::connect(c0, &Element::m_ElementChangeSignal, lambdaFun);
    auto p0A = PropertyAddress{p0->GetId(), PositionProperty, ValueRole::User};
    auto p1A = PropertyAddress{p1->GetId(), PositionProperty, ValueRole::User};
    auto s0A = PropertyAddress{s0->GetId(), LengthProperty, ValueRole::DependencyGraph};
    auto c0A = PropertyAddress{c0->GetId(), AreaProperty, ValueRole::DependencyGraph};
    m_Sketch->AddPropertyAddress(p0A);
    m_Sketch->AddPropertyAddress(p1A);
    m_Sketch->AddPropertyAddress(s0A);
    m_Sketch->AddPropertyAddress(c0A);

    m_Points.push_back(p0A);
    m_Points.push_back(p1A);
    m_Segments.push_back(s0A);
    m_Circles.push_back(c0A);

    auto pointMetaObject = m_Document->GetMetaRegister()->GetMetaObject("PointElement");
    auto pProperty = pointMetaObject->FindProperty(PositionProperty);
    pProperty->Write(p0, gp_Pnt(0, 0, 0));
    pProperty->Write(p1, gp_Pnt(100, 0, 0));
    // p0->SetProperty(PositionProperty, gp_Pnt(0, 0, 0));
    // p1->SetProperty(PositionProperty, gp_Pnt(100, 0, 0));

    m_Sketch->AddSegmentComputerNode({p0A, p1A}, {s0A});
    m_Sketch->AddCircleAreaComputerNode({s0A}, {c0A});
    m_Sketch->MarkDirty(p0A);
}

void DependencyGraphManager::EvaluateAndRefreshScene() const {
    if (!m_Sketch) {
        return;
    }
    const auto result = m_Sketch->Evaluate();
}


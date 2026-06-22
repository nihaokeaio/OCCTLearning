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
#include <utility>

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
}

void DependencyGraphManager::Test()
{
    BuildDemoGraph();

    if (!m_Points.empty())
    {
        m_Sketch->MovePoint(m_Points[0].point, gp_Pnt{10, 0, 0});
    }
    if (m_Points.size() > 2)
    {
        m_Sketch->MovePoint(m_Points[2].point, gp_Pnt{100, 200, 0});
    }

    m_Sketch->Evaluate();
}

void DependencyGraphManager::InitializeDemoScene(const Handle(AIS_InteractiveContext)& context)
{
    m_AisContext = context;
    BuildDemoGraph();
    m_Sketch->Evaluate();
    DisplayScene();
    m_ChangedValues.clear();
}

void DependencyGraphManager::RefreshScene()
{
    if (m_AisContext.IsNull() || m_Sketch == nullptr)
    {
        return;
    }

    for (auto& point : m_Points)
    {
        RefreshPoint(point);
    }
    for (auto& segment : m_Segments)
    {
        RefreshSegment(segment);
    }
    for (auto& dimension : m_DistanceDimensions)
    {
        RefreshDistanceDimension(dimension);
    }

    m_AisContext->UpdateCurrentViewer();
    m_ChangedValues.clear();
}

void DependencyGraphManager::RenderGuiControls()
{
    if (m_Sketch == nullptr)
    {
        return;
    }

    ImGui::Begin("Sketch Controls");

    bool traceEnabled = m_Context.IsTraceEnabled();
    if (ImGui::Checkbox("DGTrace", &traceEnabled))
    {
        m_Context.SetTraceEnabled(traceEnabled);
    }

    ImGui::Separator();

    for (size_t i = 0; i < m_Points.size(); ++i)
    {
        const auto position = m_Sketch->GetPosition(m_Points[i].point);
        float coords[3] = {
            static_cast<float>(position.X()),
            static_cast<float>(position.Y()),
            static_cast<float>(position.Z())
        };

        const auto label = m_Points[i].name.empty()
                               ? std::string("point ") + std::to_string(i)
                               : m_Points[i].name;
        if (ImGui::DragFloat3(label.c_str(), coords, 1.0f))
        {
            MovePoint(i, gp_Pnt{coords[0], coords[1], coords[2]});
        }
    }

    ImGui::Separator();
    for (size_t i = 0; i < m_DistanceDimensions.size(); ++i)
    {
        ImGui::Text("d%zu = %.2f", i, m_Sketch->GetDistance(m_DistanceDimensions[i].dimension));
    }

    ImGui::Separator();
    ImGui::TextUnformatted("DGFlow");
    if (m_FlowLogLines.empty())
    {
        ImGui::TextDisabled("No flow yet");
    }
    else
    {
        for (const auto& line : m_FlowLogLines)
        {
            ImGui::TextWrapped("%s", line.c_str());
        }
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Value Bindings"))
    {
        for (const auto& [valueId, externalKey] : m_Bindings.ValueToExternal())
        {
            ImGui::TextWrapped("%s [%s] -> %s",
                               m_Context.ValueLabel(valueId).c_str(),
                               ToString(m_Context.GetValueRole(valueId)),
                               externalKey.ToString().c_str());
        }
    }

    ImGui::End();
}

bool DependencyGraphManager::BeginPointDrag(const Handle(V3d_View)& view, int screenX, int screenY)
{
    size_t pointIndex = 0;
    if (!TryPickPointAtScreen(view, screenX, screenY, pointIndex))
    {
        return false;
    }

    m_DraggingPointIndex = pointIndex;
    m_TraceEnabledBeforeDrag = m_Context.IsTraceEnabled();
    m_Context.SetTraceEnabled(false);
    return DragPointTo(view, screenX, screenY);
}

bool DependencyGraphManager::DragPointTo(const Handle(V3d_View)& view, int screenX, int screenY)
{
    if (!IsDraggingPoint())
    {
        return false;
    }

    gp_Pnt position;
    if (!ProjectScreenToSketchPlane(view, screenX, screenY, position))
    {
        return false;
    }

    MovePoint(m_DraggingPointIndex, position);
    return true;
}

void DependencyGraphManager::EndPointDrag()
{
    if (IsDraggingPoint())
    {
        m_Context.SetTraceEnabled(m_TraceEnabledBeforeDrag);
    }
    m_DraggingPointIndex = static_cast<size_t>(-1);
}

bool DependencyGraphManager::IsDraggingPoint() const
{
    return m_DraggingPointIndex < m_Points.size();
}

void DependencyGraphManager::BuildDemoGraph()
{
    m_Context.Clear();
    m_Sketch = std::make_unique<SketchModel>(m_Context);
    m_Context.SetFlowTraceCallback([this](const std::vector<std::string>& lines)
    {
        m_FlowLogLines = lines;
    });

    m_Points.clear();
    m_Segments.clear();
    m_DistanceDimensions.clear();
    m_ChangedValues.clear();
    m_FlowLogLines.clear();
    m_Bindings.Clear();

    const auto j0 = m_Sketch->CreatePoint(gp_Pnt(0, 0, 0), "j0");
    const auto j1 = m_Sketch->CreatePoint(gp_Pnt(100, 0, 0), "j1");
    const auto j2 = m_Sketch->CreatePoint(gp_Pnt(100, 100, 0), "j2");
    RegisterDemoBinding(j0.position, "j0", "position");
    RegisterDemoBinding(j1.position, "j1", "position");
    RegisterDemoBinding(j2.position, "j2", "position");
    m_Points.push_back({"j0", j0, {}});
    m_Points.push_back({"j1", j1, {}});
    m_Points.push_back({"j2", j2, {}});

    const auto s0 = m_Sketch->CreateSegment(j0, j1, "s0");
    const auto s1 = m_Sketch->CreateSegment(j1, j2, "s1");
    RegisterDemoBinding(s0.length, "s0", "length");
    RegisterDemoBinding(s1.length, "s1", "length");
    m_Segments.push_back({s0, {}});
    m_Segments.push_back({s1, {}});

    const auto d0 = m_Sketch->CreateDistanceDimension(j0, j1, "d0");
    RegisterDemoBinding(d0.measuredLength, "d0", "length");
    m_DistanceDimensions.push_back({d0, {}});

    const auto rectArea = m_Sketch->CreateRectangleArea(s0, s1, "rect");
    const auto circleArea = m_Sketch->CreateCircleAreaFromRadius(s0, "circle");
    RegisterDemoBinding(rectArea.area, "rect", "area");
    RegisterDemoBinding(circleArea.area, "circle", "area");

    // 初始化阶段也走 dirty/evaluate，让派生值和真实编辑路径保持一致。
    m_Sketch->MovePoint(j0, m_Sketch->GetPosition(j0));
    m_Sketch->MovePoint(j1, m_Sketch->GetPosition(j1));
    m_Sketch->MovePoint(j2, m_Sketch->GetPosition(j2));
}

void DependencyGraphManager::EvaluateAndRefreshScene()
{
    if (m_Sketch == nullptr)
    {
        return;
    }

    const auto result = m_Sketch->Evaluate();
    if (result.evaluated && !result.changedValues.empty())
    {
        m_ChangedValues = result.changedValues;
        RefreshChangedScene();
    }
}

void DependencyGraphManager::MovePoint(size_t index, const gp_Pnt& position)
{
    if (m_Sketch == nullptr || index >= m_Points.size())
    {
        return;
    }

    m_Sketch->MovePoint(m_Points[index].point, position);
    EvaluateAndRefreshScene();
}

bool DependencyGraphManager::TryPickPointAtScreen(
    const Handle(V3d_View)& view,
    int screenX,
    int screenY,
    size_t& pointIndex) const
{
    if (view.IsNull() || m_Sketch == nullptr)
    {
        return false;
    }

    const double pickRadiusSquared = PointPickRadiusPx * PointPickRadiusPx;
    double bestDistanceSquared = pickRadiusSquared;
    bool hasPickedPoint = false;

    for (size_t i = 0; i < m_Points.size(); ++i)
    {
        const auto position = m_Sketch->GetPosition(m_Points[i].point);
        Standard_Integer pointScreenX = 0;
        Standard_Integer pointScreenY = 0;
        view->Convert(position.X(), position.Y(), position.Z(), pointScreenX, pointScreenY);

        const double dx = static_cast<double>(pointScreenX - screenX);
        const double dy = static_cast<double>(pointScreenY - screenY);
        const double distanceSquared = dx * dx + dy * dy;
        if (distanceSquared <= bestDistanceSquared)
        {
            bestDistanceSquared = distanceSquared;
            pointIndex = i;
            hasPickedPoint = true;
        }
    }

    return hasPickedPoint;
}

bool DependencyGraphManager::ProjectScreenToSketchPlane(
    const Handle(V3d_View)& view,
    int screenX,
    int screenY,
    gp_Pnt& point) const
{
    if (view.IsNull())
    {
        return false;
    }

    Standard_Real rayOriginX = 0.0;
    Standard_Real rayOriginY = 0.0;
    Standard_Real rayOriginZ = 0.0;
    Standard_Real rayDirectionX = 0.0;
    Standard_Real rayDirectionY = 0.0;
    Standard_Real rayDirectionZ = 0.0;
    view->ConvertWithProj(screenX, screenY,
                          rayOriginX, rayOriginY, rayOriginZ,
                          rayDirectionX, rayDirectionY, rayDirectionZ);

    if (std::abs(rayDirectionZ) < 1.0e-9)
    {
        return false;
    }

    const double t = (SketchPlaneZ - rayOriginZ) / rayDirectionZ;
    point = gp_Pnt{
        rayOriginX + rayDirectionX * t,
        rayOriginY + rayDirectionY * t,
        SketchPlaneZ
    };
    return true;
}

void DependencyGraphManager::RefreshChangedScene()
{
    if (m_AisContext.IsNull() || m_Sketch == nullptr)
    {
        return;
    }

    for (const auto& changedValueId : m_ChangedValues)
    {
        for (auto& point : m_Points)
        {
            if (point.point.position == changedValueId)
            {
                RefreshPoint(point);
            }
        }

        for (auto& segment : m_Segments)
        {
            if (segment.segment.length == changedValueId)
            {
                RefreshSegment(segment);
            }
        }

        for (auto& dimension : m_DistanceDimensions)
        {
            if (dimension.dimension.measuredLength == changedValueId)
            {
                RefreshDistanceDimension(dimension);
            }
        }
    }

    m_ChangedValues.clear();
    m_AisContext->UpdateCurrentViewer();
}

void DependencyGraphManager::RegisterDemoBinding(
    ValueId valueId,
    std::string objectId,
    std::string propertyKey)
{
    m_Bindings.RegisterBinding(valueId, {std::move(objectId), std::move(propertyKey)});
}

void DependencyGraphManager::DisplayScene()
{
    if (m_AisContext.IsNull())
    {
        return;
    }

    for (auto& point : m_Points)
    {
        RefreshPoint(point);
        point.shape->SetColor(Quantity_NOC_TOMATO);
        m_AisContext->Display(point.shape, AIS_Shaded, 0, false);
    }

    for (auto& segment : m_Segments)
    {
        RefreshSegment(segment);
        segment.shape->SetColor(Quantity_Color(0.1, 0.45, 0.9, Quantity_TOC_RGB));
        m_AisContext->Display(segment.shape, AIS_WireFrame, 0, false);
    }

    for (auto& dimension : m_DistanceDimensions)
    {
        RefreshDistanceDimension(dimension);
        dimension.label->SetColor(Quantity_NOC_GOLD);
        m_AisContext->Display(dimension.label, false);
    }

    m_AisContext->UpdateCurrentViewer();
    m_ChangedValues.clear();
}

void DependencyGraphManager::RefreshPoint(RenderedPoint& renderedPoint)
{
    const auto position = m_Sketch->GetPosition(renderedPoint.point);
    const auto shape = MakePointShape(position);

    if (renderedPoint.shape.IsNull())
    {
        renderedPoint.shape = new AIS_Shape(shape);
        return;
    }
    renderedPoint.shape->SetShape(shape);
    m_AisContext->Redisplay(renderedPoint.shape, false);
}

void DependencyGraphManager::RefreshSegment(RenderedSegment& renderedSegment)
{
    const auto start = m_Sketch->GetPosition(renderedSegment.segment.start);
    const auto end = m_Sketch->GetPosition(renderedSegment.segment.end);
    const auto shape = MakeSegmentShape(start, end);

    if (renderedSegment.shape.IsNull())
    {
        renderedSegment.shape = new AIS_Shape(shape);
        return;
    }
    renderedSegment.shape->SetShape(shape);
    m_AisContext->Redisplay(renderedSegment.shape, false);
}

void DependencyGraphManager::RefreshDistanceDimension(RenderedDistanceDimension& renderedDimension)
{
    const auto start = m_Sketch->GetPosition(renderedDimension.dimension.start);
    const auto end = m_Sketch->GetPosition(renderedDimension.dimension.end);
    auto position = MidPoint(start, end);
    position.SetY(position.Y() + DimensionTextOffset);

    const auto text = FormatDistance(m_Sketch->GetDistance(renderedDimension.dimension));

    const bool isNewLabel = renderedDimension.label.IsNull();
    if (renderedDimension.label.IsNull())
    {
        renderedDimension.label = new AIS_TextLabel();
        renderedDimension.label->SetHeight(16.0);
    }
    renderedDimension.label->SetPosition(position);
    renderedDimension.label->SetText(TCollection_ExtendedString(text.c_str()));
    if (!isNewLabel)
    {
        m_AisContext->Redisplay(renderedDimension.label, false);
    }
}

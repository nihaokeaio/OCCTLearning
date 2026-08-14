#include "DependencyGraphManager.h"

#include "imgui/imgui.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopoDS_Shape.hxx>

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>

#include "Data/Element.h"
#include "Data/Document.h"
#include "Data/Property/PropertyResolver.h"
#include "Demo/SketchRuntime.h"
#include "Demo/domain/GeneralComputer.h"

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
    constexpr auto TotalLengthProperty = "TotalLength";

    std::string FormatAddress(const PropertyAddress& address)
    {
        return address.elementId.ToString() + "." + address.propertyName;
    }
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
    m_Circles.clear();
    m_Metrics.clear();

    auto p0 = sketch->CreateElement("PointElement");
    auto p1 = sketch->CreateElement("PointElement");
    auto p2 = sketch->CreateElement("PointElement");
    auto s0 = sketch->CreateElement("SegmentElement");
    auto s1 = sketch->CreateElement("SegmentElement");
    auto c0 = sketch->CreateElement("CircleElement");
    auto c1 = sketch->CreateElement("CircleElement");
    auto metrics = sketch->CreateElement("MetricsElement");
    auto lambdaFun = [this](const MessageInfo::PropertyChangePayload& message)
    {
        printf("Element [%s] Property change! [key]= %s \n", message.address.elementId.ToString().c_str(),
               message.address.propertyName.c_str());
    };
    MiniSignal::connect(m_SketchRuntime->GetDocument(), &Document::m_ElementPropertyChangedSignal, lambdaFun);
    auto p0A = PropertyAddress{p0->GetId(), PositionProperty};
    auto p1A = PropertyAddress{p1->GetId(), PositionProperty};
    auto p2A = PropertyAddress{p2->GetId(), PositionProperty};
    auto s0A = PropertyAddress{s0->GetId(), LengthProperty};
    auto s1A = PropertyAddress{s1->GetId(), LengthProperty};
    auto c0A = PropertyAddress{c0->GetId(), AreaProperty};
    auto c1A = PropertyAddress{c1->GetId(), AreaProperty};
    auto totalLengthA = PropertyAddress{metrics->GetId(), TotalLengthProperty};
    const PropertyAddress addresses[] = {p0A, p1A, p2A, s0A, s1A, c0A, c1A, totalLengthA};
    for (const auto& address : addresses)
    {
        sketch->AddPropertyAddress(address);
    }

    m_Points.push_back(p0A);
    m_Points.push_back(p1A);
    m_Points.push_back(p2A);
    m_Segments.push_back(s0A);
    m_Segments.push_back(s1A);
    m_Circles.push_back(c0A);
    m_Circles.push_back(c1A);
    m_Metrics.push_back(totalLengthA);

    m_SketchRuntime->SetProperty(p0A, gp_Pnt(0, 0, 0), ChangeSource::User);
    m_SketchRuntime->SetProperty(p1A, gp_Pnt(100, 0, 0), ChangeSource::User);
    m_SketchRuntime->SetProperty(p2A, gp_Pnt(100, 80, 0), ChangeSource::User);
    m_PointEditorPositions = {{{0.0F, 0.0F, 0.0F}, {100.0F, 0.0F, 0.0F}, {100.0F, 80.0F, 0.0F}}};
    m_PointEditorDirty.fill(false);

    sketch->AddComputerNode({p0A, p1A}, {s0A}, DistanceComputer{}, "Segment0.Distance");
    sketch->AddComputerNode({p1A, p2A}, {s1A}, DistanceComputer{}, "Segment1.Distance");
    sketch->AddComputerNode({s0A}, {c0A}, CircleAreaComputer{}, "Circle0.Area");
    sketch->AddComputerNode({s1A}, {c1A}, CircleAreaComputer{}, "Circle1.Area");
    sketch->AddComputerNode({s0A, s1A}, {totalLengthA}, SumComputer{}, "Metrics.TotalLength");
}

void DependencyGraphManager::EvaluateAndRefreshScene()
{
    m_LastEvaluationResult = m_SketchRuntime->Flush();
    PrintEvaluationLog();
}

void DependencyGraphManager::DrawImGui()
{
    if (!ImGui::Begin("Dependency Graph Runtime"))
    {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("Edit several points, then apply them in one Flush.");
    for (std::size_t index = 0; index < m_PointEditorPositions.size(); ++index)
    {
        const auto label = "P" + std::to_string(index) + ".Position";
        if (ImGui::DragFloat3(label.c_str(), m_PointEditorPositions[index].data(), 1.0F))
        {
            m_PointEditorDirty[index] = true;
        }
    }

    if (ImGui::Button("Apply pending edits + Flush"))
    {
        bool changed = false;
        for (std::size_t index = 0; index < m_Points.size() && index < m_PointEditorDirty.size(); ++index)
        {
            if (!m_PointEditorDirty[index])
            {
                continue;
            }
            const auto& position = m_PointEditorPositions[index];
            changed |= m_SketchRuntime->SetProperty(
                m_Points[index], gp_Pnt{position[0], position[1], position[2]}, ChangeSource::User);
            m_PointEditorDirty[index] = false;
        }
        if (changed)
        {
            EvaluateAndRefreshScene();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Flush without edits"))
    {
        EvaluateAndRefreshScene();
    }

    const auto readDouble = [this](const PropertyAddress& address) -> double
    {
        const auto value = m_SketchRuntime->GetPropertyResolver()->Read(address);
        if (!value)
        {
            return 0.0;
        }
        const auto* number = value->GetIf<double>();
        return number ? *number : 0.0;
    };

    ImGui::SeparatorText("Derived values");
    for (std::size_t index = 0; index < m_Segments.size(); ++index)
    {
        ImGui::Text("Segment%zu.Length = %.3f", index, readDouble(m_Segments[index]));
    }
    for (std::size_t index = 0; index < m_Circles.size(); ++index)
    {
        ImGui::Text("Circle%zu.Area = %.3f", index, readDouble(m_Circles[index]));
    }
    if (!m_Metrics.empty())
    {
        ImGui::Text("Metrics.TotalLength = %.3f", readDouble(m_Metrics.front()));
    }

    ImGui::SeparatorText("Last Flush");
    ImGui::Text("Evaluated: %s", m_LastEvaluationResult.evaluated ? "yes" : "no");
    ImGui::Text("Executed nodes: %zu", m_LastEvaluationResult.nodeTraces.size());
    ImGui::Text("Changed values: %zu", m_LastEvaluationResult.changedValues.size());

    if (ImGui::TreeNode("Changed value addresses"))
    {
        for (const auto& address : m_LastEvaluationResult.changedValues)
        {
            const auto addressText = FormatAddress(address);
            ImGui::BulletText("%s", addressText.c_str());
        }
        ImGui::TreePop();
    }

    for (const auto& trace : m_LastEvaluationResult.nodeTraces)
    {
        const auto nodeName = trace.nodeName.empty() ? trace.nodeId.ToString() : trace.nodeName;
        ImGui::BulletText("Batch %zu: %s", trace.batchIndex, nodeName.c_str());
        ImGui::Indent();
        for (const auto& trigger : trace.triggeredBy)
        {
            const auto addressText = FormatAddress(trigger);
            ImGui::Text("trigger: %s", addressText.c_str());
        }
        for (const auto& output : trace.outputs)
        {
            const auto addressText = FormatAddress(output);
            ImGui::Text("output:  %s", addressText.c_str());
        }
        ImGui::Unindent();
    }

    ImGui::End();
}

void DependencyGraphManager::PrintEvaluationLog() const
{
    std::printf("[DG] evaluated=%s, nodes=%zu, changedValues=%zu\n",
                m_LastEvaluationResult.evaluated ? "true" : "false",
                m_LastEvaluationResult.nodeTraces.size(),
                m_LastEvaluationResult.changedValues.size());
    for (const auto& trace : m_LastEvaluationResult.nodeTraces)
    {
        const auto nodeName = trace.nodeName.empty() ? trace.nodeId.ToString() : trace.nodeName;
        std::printf("[DG] batch=%zu node=%s triggers=%zu outputs=%zu\n",
                    trace.batchIndex, nodeName.c_str(), trace.triggeredBy.size(), trace.outputs.size());
        for (const auto& trigger : trace.triggeredBy)
        {
            const auto addressText = FormatAddress(trigger);
            std::printf("     trigger %s\n", addressText.c_str());
        }
        for (const auto& output : trace.outputs)
        {
            const auto addressText = FormatAddress(output);
            std::printf("     output  %s\n", addressText.c_str());
        }
    }
    for (const auto& changedValue : m_LastEvaluationResult.changedValues)
    {
        const auto addressText = FormatAddress(changedValue);
        std::printf("[DG] changed %s\n", addressText.c_str());
    }
}


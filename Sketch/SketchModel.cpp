#include "SketchModel.h"

#include "DependencyGraph/ComputerView.h"

SketchModel::SketchModel(DGContext& context)
    : m_Context(context)
{
}

SketchPoint SketchModel::CreatePoint(const gp_Pnt& position, const std::string& debugName)
{
    SketchPoint point{m_Context.CreateValue(PositionProperty, position)};
    if (!debugName.empty())
    {
        m_Context.SetDebugName(point.position, debugName + ".position");
    }
    return point;
}

SketchSegment SketchModel::CreateSegment(const SketchPoint& start, const SketchPoint& end,
                                         const std::string& debugName)
{
    SketchSegment segment{
        start,
        end,
        m_Context.CreateValue(LengthProperty, 0.0),
        {}
    };

    segment.lengthNode = m_Context.AddComputeNode({start.position, end.position}, {segment.length},
        [](const ComputerView& view)
        {
            const auto p0 = view.Input<gp_Pnt>(0, PositionProperty);
            const auto p1 = view.Input<gp_Pnt>(1, PositionProperty);
            view.SetOutput(0, LengthProperty, p0.Distance(p1));
        });

    if (!debugName.empty())
    {
        m_Context.SetDebugName(segment.length, debugName + ".length");
        m_Context.SetDebugName(segment.lengthNode, "distance " + debugName);
    }
    return segment;
}

SketchArea SketchModel::CreateRectangleArea(const SketchSegment& width, const SketchSegment& height,
                                            const std::string& debugName)
{
    SketchArea area{
        m_Context.CreateValue(AreaProperty, 0.0),
        {}
    };

    area.computeNode = m_Context.AddComputeNode({width.length, height.length}, {area.area},
        [](const ComputerView& view)
        {
            const auto widthLength = view.Input<double>(0, LengthProperty);
            const auto heightLength = view.Input<double>(1, LengthProperty);
            view.SetOutput(0, AreaProperty, widthLength * heightLength);
        });

    if (!debugName.empty())
    {
        m_Context.SetDebugName(area.area, debugName + ".area");
        m_Context.SetDebugName(area.computeNode, debugName + " area");
    }
    return area;
}

SketchArea SketchModel::CreateCircleAreaFromRadius(const SketchSegment& radius,
                                                   const std::string& debugName)
{
    SketchArea area{
        m_Context.CreateValue(AreaProperty, 0.0),
        {}
    };

    area.computeNode = m_Context.AddComputeNode({radius.length}, {area.area},
        [](const ComputerView& view)
        {
            constexpr double pi = 3.14159265358979323846;
            const auto radiusLength = view.Input<double>(0, LengthProperty);
            view.SetOutput(0, AreaProperty, pi * radiusLength * radiusLength);
        });

    if (!debugName.empty())
    {
        m_Context.SetDebugName(area.area, debugName + ".area");
        m_Context.SetDebugName(area.computeNode, debugName + " area");
    }
    return area;
}

ComputerNodeId SketchModel::AddLengthRenderNode(const SketchSegment& segment, const std::string& debugName)
{
    const auto renderNode = m_Context.AddComputeNode({segment.length}, {},
        [this](const ComputerView& view)
        {
            m_Context.GetRender()->Update(view.InId(0));
        });

    if (!debugName.empty())
    {
        m_Context.SetDebugName(renderNode, "render " + debugName);
    }
    return renderNode;
}

void SketchModel::MovePoint(const SketchPoint& point, const gp_Pnt& position)
{
    m_Context.SetValueProperty(point.position, PositionProperty, position);
}

bool SketchModel::Evaluate()
{
    return m_Context.Evaluator();
}

gp_Pnt SketchModel::GetPosition(const SketchPoint& point) const
{
    return m_Context.GetValueHandle(point.position).GetProperty<gp_Pnt>(PositionProperty);
}

double SketchModel::GetLength(const SketchSegment& segment) const
{
    return m_Context.GetValueHandle(segment.length).GetProperty<double>(LengthProperty);
}

double SketchModel::GetArea(const SketchArea& area) const
{
    return m_Context.GetValueHandle(area.area).GetProperty<double>(AreaProperty);
}

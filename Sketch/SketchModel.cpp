#include "SketchModel.h"

#include "DependencyGraph/Core/ComputerView.h"

SketchModel::SketchModel(DGContext& context)
    : m_Context(context)
{
}

SketchPoint SketchModel::CreatePoint(const gp_Pnt& position, const std::string& debugName)
{
    SketchPoint point{m_Context.CreateInputValue(PositionProperty, position)};
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
        m_Context.CreateDerivedValue(LengthProperty, 0.0),
        {}
    };

    segment.lengthNode = AddDistanceComputeNode(start.position, end.position, segment.length);

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
        m_Context.CreateDerivedValue(AreaProperty, 0.0),
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
        m_Context.CreateDerivedValue(AreaProperty, 0.0),
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

SketchDistanceDimension SketchModel::CreateDistanceDimension(const SketchPoint& start, const SketchPoint& end,
                                                             const std::string& debugName)
{
    SketchDistanceDimension dimension{
        start,
        end,
        m_Context.CreateDerivedValue(LengthProperty, 0.0),
        {}
    };

    dimension.computeNode = AddDistanceComputeNode(start.position, end.position, dimension.measuredLength);

    if (!debugName.empty())
    {
        m_Context.SetDebugName(dimension.measuredLength, debugName + ".length");
        m_Context.SetDebugName(dimension.computeNode, "distance " + debugName);
    }
    return dimension;
}

double SketchModel::GetDistance(const SketchDistanceDimension& dimension) const
{
    return m_Context.GetValueProperty<double>(dimension.measuredLength, LengthProperty);
}

void SketchModel::MovePoint(const SketchPoint& point, const gp_Pnt& position)
{
    m_Context.SetInputValueProperty(point.position, PositionProperty, position);
}

DGContext::EvaluationResult SketchModel::Evaluate()
{
    return m_Context.Evaluate();
}

gp_Pnt SketchModel::GetPosition(const SketchPoint& point) const
{
    return m_Context.GetValueProperty<gp_Pnt>(point.position, PositionProperty);
}

double SketchModel::GetLength(const SketchSegment& segment) const
{
    return m_Context.GetValueProperty<double>(segment.length, LengthProperty);
}

double SketchModel::GetArea(const SketchArea& area) const
{
    return m_Context.GetValueProperty<double>(area.area, AreaProperty);
}

ComputerNodeId SketchModel::AddDistanceComputeNode(ValueId startPosition, ValueId endPosition, ValueId outputLength)
{
    return m_Context.AddComputeNode({startPosition, endPosition}, {outputLength},
        [](const ComputerView& view)
        {
            const auto p0 = view.Input<gp_Pnt>(0, PositionProperty);
            const auto p1 = view.Input<gp_Pnt>(1, PositionProperty);
            view.SetOutput(0, LengthProperty, p0.Distance(p1));
        });
}

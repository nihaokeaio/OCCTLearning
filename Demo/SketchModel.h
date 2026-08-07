#pragma once

#include "../DependencyGraph/DGContext.h"

#include <gp_Pnt.hxx>

#include <string>

struct SketchPoint
{
    ValueId position;
};

struct SketchSegment
{
    SketchPoint start;
    SketchPoint end;
    ValueId length;
    ComputerNodeId lengthNode;
};

struct SketchArea
{
    ValueId area;
    ComputerNodeId computeNode;
};

// 业务层对象
struct SketchDistanceDimension
{
    SketchPoint start;
    SketchPoint end;
    ValueId measuredLength;
    ComputerNodeId computeNode;
};

class SketchModel
{
public:
    explicit SketchModel(DGContext& context);

    SketchPoint CreatePoint(const gp_Pnt& position, const std::string& debugName = {});
    SketchSegment CreateSegment(const SketchPoint& start, const SketchPoint& end,
                                const std::string& debugName = {});
    SketchArea CreateRectangleArea(const SketchSegment& width, const SketchSegment& height,
                                   const std::string& debugName = {});
    SketchArea CreateCircleAreaFromRadius(const SketchSegment& radius,
                                          const std::string& debugName = {});

    // 业务层对象
    SketchDistanceDimension CreateDistanceDimension(const SketchPoint& start, const SketchPoint& end,
                                                    const std::string& debugName = {});
    [[nodiscard]] double GetDistance(const SketchDistanceDimension& dimension) const;

    void MovePoint(const SketchPoint& point, const gp_Pnt& position);
    DGContext::EvaluationResult Evaluate();

    [[nodiscard]] gp_Pnt GetPosition(const SketchPoint& point) const;
    [[nodiscard]] double GetLength(const SketchSegment& segment) const;
    [[nodiscard]] double GetArea(const SketchArea& area) const;

private:
    ComputerNodeId AddDistanceComputeNode(ValueId startPosition, ValueId endPosition, ValueId outputLength);

    static constexpr const char* PositionProperty = "position";
    static constexpr const char* LengthProperty = "length";
    static constexpr const char* AreaProperty = "area";

    DGContext& m_Context;
};

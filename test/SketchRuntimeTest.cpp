//
// Created by ZQD on 26-8-13.
//
#include <gtest/gtest.h>
#include "SketchRuntimeBaseFixture.h"
#include "Demo/SketchModel.h"
#include "Demo/SketchRuntime.h"
#include "Data/Element.h"
#include "Data/Property/PropertyResolver.h"
#include <gp_Pnt.hxx>
#include <algorithm>
#include <numbers>

#include "Demo/domain/GeneralComputer.h"

namespace
{
    struct BranchedGraphScene
    {
        PropertyAddress p0Position;
        PropertyAddress p1Position;
        PropertyAddress p2Position;
        PropertyAddress segment0Length;
        PropertyAddress segment1Length;
        PropertyAddress circle0Area;
        PropertyAddress circle1Area;
        PropertyAddress totalLength;
    };

    class BranchedDependencyGraphFixture : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            auto* sketch = runtime.GetSketchModel();

            auto* p0 = sketch->CreateElement("PointElement");
            auto* p1 = sketch->CreateElement("PointElement");
            auto* p2 = sketch->CreateElement("PointElement");
            auto* s0 = sketch->CreateElement("SegmentElement");
            auto* s1 = sketch->CreateElement("SegmentElement");
            auto* c0 = sketch->CreateElement("CircleElement");
            auto* c1 = sketch->CreateElement("CircleElement");
            auto* metrics = sketch->CreateElement("MetricsElement");

            scene = BranchedGraphScene{
                {p0->GetId(), "Position"},
                {p1->GetId(), "Position"},
                {p2->GetId(), "Position"},
                {s0->GetId(), "Length"},
                {s1->GetId(), "Length"},
                {c0->GetId(), "Area"},
                {c1->GetId(), "Area"},
                {metrics->GetId(), "TotalLength"}
            };

            const PropertyAddress addresses[] = {
                scene->p0Position,
                scene->p1Position,
                scene->p2Position,
                scene->segment0Length,
                scene->segment1Length,
                scene->circle0Area,
                scene->circle1Area,
                scene->totalLength
            };
            for (const auto& address : addresses)
            {
                sketch->AddPropertyAddress(address);
            }

            sketch->AddComputerNode(
                {scene->p0Position, scene->p1Position}, {scene->segment0Length},
                DistanceComputer{}, "Segment0.Distance");
            sketch->AddComputerNode(
                {scene->p1Position, scene->p2Position}, {scene->segment1Length},
                DistanceComputer{}, "Segment1.Distance");
            sketch->AddComputerNode(
                {scene->segment0Length}, {scene->circle0Area},
                CircleAreaComputer{}, "Circle0.Area");
            sketch->AddComputerNode(
                {scene->segment1Length}, {scene->circle1Area},
                CircleAreaComputer{}, "Circle1.Area");
            sketch->AddComputerNode(
                {scene->segment0Length, scene->segment1Length}, {scene->totalLength},
                SumComputer{}, "Metrics.TotalLength");

            SetPoints({0, 0, 0}, {100, 0, 0}, {100, 80, 0});
            ASSERT_TRUE(runtime.Flush().evaluated);
        }

        void SetPoints(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2)
        {
            ASSERT_TRUE(runtime.SetProperty(scene->p0Position, p0, ChangeSource::User));
            ASSERT_TRUE(runtime.SetProperty(scene->p1Position, p1, ChangeSource::User));
            ASSERT_TRUE(runtime.SetProperty(scene->p2Position, p2, ChangeSource::User));
        }

        [[nodiscard]] static std::size_t CountExecutions(
            const GraphExecutor::EvaluationResult& result, const std::string_view nodeName)
        {
            return static_cast<std::size_t>(std::ranges::count_if(
                result.nodeTraces,
                [nodeName](const GraphExecutor::NodeEvaluationTrace& trace)
                {
                    return trace.nodeName == nodeName;
                }));
        }

        [[nodiscard]] double ReadDouble(const PropertyAddress& address) const
        {
            const auto value = runtime.GetPropertyResolver()->Read(address);
            EXPECT_TRUE(value.has_value());
            if (!value)
            {
                return 0.0;
            }
            const auto* number = value->GetIf<double>();
            EXPECT_NE(number, nullptr);
            return number ? *number : 0.0;
        }

        SketchRuntime runtime;
        std::optional<BranchedGraphScene> scene;
    };
}


TEST(SketchRuntimeTest, UserPropertyChangePropagatesThroughDependencyGraph)
{
    SketchRuntime runtime;

    auto* sketch = runtime.GetSketchModel();
    auto* resolver = runtime.GetPropertyResolver();

    auto* p0 = sketch->CreateElement("PointElement");
    auto* p1 = sketch->CreateElement("PointElement");
    auto* s0 = sketch->CreateElement("SegmentElement");
    auto* c0 = sketch->CreateElement("CircleElement");

    PropertyAddress p0Position{p0->GetId(), "Position"};
    PropertyAddress p1Position{p1->GetId(), "Position"};
    PropertyAddress s0Length{s0->GetId(), "Length"};
    PropertyAddress c0Area{c0->GetId(), "Area"};

    sketch->AddPropertyAddress(p0Position);
    sketch->AddPropertyAddress(p1Position);
    sketch->AddPropertyAddress(s0Length);
    sketch->AddPropertyAddress(c0Area);

    sketch->AddComputerNode({p0Position, p1Position}, {s0Length}, DistanceComputer());
    sketch->AddComputerNode({s0Length}, {c0Area}, CircleAreaComputer());

    EXPECT_TRUE(runtime.SetProperty(p0Position,gp_Pnt{0, 0, 0},ChangeSource::User));

    EXPECT_TRUE(runtime.SetProperty(p1Position,gp_Pnt{100, 0, 0},ChangeSource::User));

    const auto result = runtime.Flush();

    const auto lengthValue = resolver->Read(s0Length);
    ASSERT_TRUE(lengthValue.has_value());

    const double* length = lengthValue->GetIf<double>();
    ASSERT_NE(length, nullptr);
    EXPECT_DOUBLE_EQ(*length, 100.0);

    const auto areaValue = resolver->Read(c0Area);
    ASSERT_TRUE(areaValue.has_value());

    const double* area = areaValue->GetIf<double>();
    ASSERT_NE(area, nullptr);
    EXPECT_NEAR(*area, std::numbers::pi * 100.0 * 100.0, 1e-8);

    EXPECT_TRUE(result.evaluated);
    EXPECT_TRUE(result.changedValues.contains(s0Length));
    EXPECT_TRUE(result.changedValues.contains(c0Area));
}

TEST_F(SketchRuntimeBaseFixture, FlushWithoutNewChangesDoesNothing)
{
    SetEndPoint();
    const auto result1 = runtime.Flush();
    EXPECT_TRUE(result1.evaluated);
    EXPECT_TRUE(result1.changedValues.contains(scene->length));
    EXPECT_TRUE(result1.changedValues.contains(scene->area));

    // 再次更新
    const auto result2 = runtime.Flush();
    EXPECT_FALSE(result2.evaluated);
    EXPECT_TRUE(result2.changedValues.empty());
    EXPECT_TRUE(result2.nodeTraces.empty());
}

TEST_F(SketchRuntimeBaseFixture, ComputedWritesAreReportedAsDependencyGraphSource)
{
    std::vector<MessageInfo::PropertyChangePayload> events;
    MiniSignal::connect(runtime.GetDocument(), &Document::m_ElementPropertyChangedSignal,
                        [&events](const MessageInfo::PropertyChangePayload& message)
                        {
                            events.emplace_back(message);
                        });
    SetEndPoint();
    const auto result = runtime.Flush();
    EXPECT_TRUE(result.evaluated);
    EXPECT_TRUE(result.changedValues.contains(scene->length));
    EXPECT_TRUE(result.changedValues.contains(scene->area));

    EXPECT_EQ(events.size(), 4);
    for (const auto& event : events)
    {
        if (event.address == scene->p0Position || event.address == scene->p1Position)
        {
            EXPECT_EQ(event.source, ChangeSource::User);
        }

        if (event.address == scene->length || event.address == scene->area)
        {
            EXPECT_EQ(event.source, ChangeSource::DependencyGraph);
        }
    }
}

TEST_F(BranchedDependencyGraphFixture, SharedPointRecomputesBothBranchesAndMergeOnce)
{
    ASSERT_TRUE(runtime.SetProperty(scene->p1Position, gp_Pnt{120, 20, 0}, ChangeSource::User));

    const auto result = runtime.Flush();

    EXPECT_EQ(CountExecutions(result, "Segment0.Distance"), 1);
    EXPECT_EQ(CountExecutions(result, "Segment1.Distance"), 1);
    EXPECT_EQ(CountExecutions(result, "Circle0.Area"), 1);
    EXPECT_EQ(CountExecutions(result, "Circle1.Area"), 1);
    EXPECT_EQ(CountExecutions(result, "Metrics.TotalLength"), 1);
}

TEST_F(BranchedDependencyGraphFixture, BatchedEndpointChangesDeduplicateMergeNode)
{
    ASSERT_TRUE(runtime.SetProperty(scene->p0Position, gp_Pnt{-20, 0, 0}, ChangeSource::User));
    ASSERT_TRUE(runtime.SetProperty(scene->p2Position, gp_Pnt{100, 100, 0}, ChangeSource::User));

    const auto result = runtime.Flush();

    EXPECT_EQ(CountExecutions(result, "Segment0.Distance"), 1);
    EXPECT_EQ(CountExecutions(result, "Segment1.Distance"), 1);
    EXPECT_EQ(CountExecutions(result, "Metrics.TotalLength"), 1);
    const auto totalTrace = std::ranges::find_if(
        result.nodeTraces,
        [](const GraphExecutor::NodeEvaluationTrace& trace)
        {
            return trace.nodeName == "Metrics.TotalLength";
        });
    ASSERT_NE(totalTrace, result.nodeTraces.end());
    EXPECT_EQ(totalTrace->triggeredBy.size(), 2);
    EXPECT_NE(std::ranges::find(totalTrace->triggeredBy, scene->segment0Length), totalTrace->triggeredBy.end());
    EXPECT_NE(std::ranges::find(totalTrace->triggeredBy, scene->segment1Length), totalTrace->triggeredBy.end());
    EXPECT_DOUBLE_EQ(ReadDouble(scene->totalLength), 220.0);
}

TEST_F(BranchedDependencyGraphFixture, SingleEndpointChangeSkipsUnrelatedBranch)
{
    ASSERT_TRUE(runtime.SetProperty(scene->p0Position, gp_Pnt{-20, 0, 0}, ChangeSource::User));

    const auto result = runtime.Flush();

    EXPECT_EQ(CountExecutions(result, "Segment0.Distance"), 1);
    EXPECT_EQ(CountExecutions(result, "Circle0.Area"), 1);
    EXPECT_EQ(CountExecutions(result, "Metrics.TotalLength"), 1);
    EXPECT_EQ(CountExecutions(result, "Segment1.Distance"), 0);
    EXPECT_EQ(CountExecutions(result, "Circle1.Area"), 0);
    EXPECT_DOUBLE_EQ(ReadDouble(scene->totalLength), 200.0);
}

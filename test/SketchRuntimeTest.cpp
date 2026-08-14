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
#include <numbers>

#include "Demo/domain/GeneralComputer.h"


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
    const auto [evaluated, changedValues] = runtime.Flush();
    EXPECT_TRUE(evaluated);
    EXPECT_TRUE(changedValues.contains(scene->length));
    EXPECT_TRUE(changedValues.contains(scene->area));

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

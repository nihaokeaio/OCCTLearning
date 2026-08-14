//
// Created by ZQD on 26-8-13.
//
#include "SketchRuntimeBaseFixture.h"

#include "Data/Element.h"
#include "Demo/SketchModel.h"
#include "Demo/SketchRuntime.h"
#include "Data/Property/PropertyResolver.h"
#include "Demo/domain/GeneralComputer.h"

void SketchRuntimeBaseFixture::SetUp()
{
    const auto* sketch = runtime.GetSketchModel();

    auto* p0 = sketch->CreateElement("PointElement");
    auto* p1 = sketch->CreateElement("PointElement");
    auto* s0 = sketch->CreateElement("SegmentElement");
    auto* c0 = sketch->CreateElement("CircleElement");

    PropertyAddress p0Position{p0->GetId(), "Position"};
    PropertyAddress p1Position{p1->GetId(), "Position"};
    PropertyAddress s0Length{s0->GetId(), "Length"};
    PropertyAddress c0Area{c0->GetId(), "Area"};
    scene = {
        p0Position,
        p1Position,
        s0Length,
        c0Area
    };

    sketch->AddPropertyAddress(p0Position);
    sketch->AddPropertyAddress(p1Position);
    sketch->AddPropertyAddress(s0Length);
    sketch->AddPropertyAddress(c0Area);
    sketch->AddComputerNode({p0Position, p1Position}, {s0Length}, DistanceComputer());
    sketch->AddComputerNode({s0Length}, {c0Area}, CircleAreaComputer());
}

void SketchRuntimeBaseFixture::SetEndPoint(gp_Pnt p0, gp_Pnt p1) const
{
    ASSERT_TRUE(runtime.SetProperty(scene->p0Position, p0, ChangeSource::User));
    ASSERT_TRUE(runtime.SetProperty(scene->p1Position, p1, ChangeSource::User));
}


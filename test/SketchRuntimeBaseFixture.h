//
// Created by ZQD on 26-8-13.
//

#pragma once
#include <gp_Pnt.hxx>
#include <gtest/gtest.h>

#include "Data/Property/PropertyAddress.h"
#include "Demo/SketchRuntime.h"

struct LengthAreaScene
{
    PropertyAddress p0Position;
    PropertyAddress p1Position;
    PropertyAddress length;
    PropertyAddress area;
};

class SketchRuntimeBaseFixture : public ::testing::Test
{
protected:
    void SetUp() override;

    void SetEndPoint(gp_Pnt p0 = {10, 0, 0}, gp_Pnt p1 = {110, 0, 0}) const;
    SketchRuntime runtime;
    std::optional<LengthAreaScene> scene;
};




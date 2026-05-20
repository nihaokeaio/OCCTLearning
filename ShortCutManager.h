//
// Created by ZQD on 26-5-18.
//

#pragma once
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_Handle.hxx>
#include <V3d_View.hxx>


class ShortCutManager
{
public:
    ShortCutManager(Handle(V3d_View) view,Handle(OpenGl_GraphicDriver) graphicDriver);

    bool IsPointVisible(const gp_Pnt& point) const;

    static void SaveDepthImage(const std::vector<float>& depthBuffer, int width, int height);

    void Test();

private:
    Handle(V3d_View) m_RawView;
    Handle(OpenGl_GraphicDriver) m_GraphicDriver;
};




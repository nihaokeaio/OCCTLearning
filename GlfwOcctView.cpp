// MIT License
// 
// Copyright(c) 2023 Shing Liu
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "GlfwOcctView.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <BRepBUilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

#include <iostream>

#include <GLFW/glfw3.h>

namespace
{
    //! Convert GLFW mouse button into Aspect_VKeyMouse.
    static Aspect_VKeyMouse mouseButtonFromGlfw(int theButton)
    {
        switch (theButton)
        {
        case GLFW_MOUSE_BUTTON_LEFT:   return Aspect_VKeyMouse_LeftButton;
        case GLFW_MOUSE_BUTTON_RIGHT:  return Aspect_VKeyMouse_RightButton;
        case GLFW_MOUSE_BUTTON_MIDDLE: return Aspect_VKeyMouse_MiddleButton;
        }
        return Aspect_VKeyMouse_NONE;
    }

    //! Convert GLFW key modifiers into Aspect_VKeyFlags.
    static Aspect_VKeyFlags keyFlagsFromGlfw(int theFlags)
    {
        Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
        if ((theFlags & GLFW_MOD_SHIFT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_SHIFT;
        }
        if ((theFlags & GLFW_MOD_CONTROL) != 0)
        {
            aFlags |= Aspect_VKeyFlags_CTRL;
        }
        if ((theFlags & GLFW_MOD_ALT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_ALT;
        }
        if ((theFlags & GLFW_MOD_SUPER) != 0)
        {
            aFlags |= Aspect_VKeyFlags_META;
        }
        return aFlags;
    }
}

// ================================================================
// Function : GlfwOcctView
// Purpose  :
// ================================================================
GlfwOcctView::GlfwOcctView()
{
}

// ================================================================
// Function : ~GlfwOcctView
// Purpose  :
// ================================================================
GlfwOcctView::~GlfwOcctView()
{
}

// ================================================================
// Function : toView
// Purpose  :
// ================================================================
GlfwOcctView* GlfwOcctView::toView(GLFWwindow* theWin)
{
    return static_cast<GlfwOcctView*>(glfwGetWindowUserPointer(theWin));
}

// ================================================================
// Function : errorCallback
// Purpose  :
// ================================================================
void GlfwOcctView::errorCallback(int theError, const char* theDescription)
{
    Message::DefaultMessenger()->Send(TCollection_AsciiString("Error") + theError + ": " + theDescription, Message_Fail);
}

// ================================================================
// Function : run
// Purpose  :
// ================================================================
void GlfwOcctView::run()
{
    initWindow(800, 600, "OCCT IMGUI");
    initViewer();
    initDemoScene();
    if (myView.IsNull())
    {
        return;
    }

    myView->MustBeResized();
    myOcctWindow->Map();
    initGui();
    mainloop();
    cleanup();
}

// ================================================================
// Function : initWindow
// Purpose  :
// ================================================================
void GlfwOcctView::initWindow(int theWidth, int theHeight, const char* theTitle)
{
    glfwSetErrorCallback(GlfwOcctView::errorCallback);
    glfwInit();
    const bool toAskCoreProfile = true;
    if (toAskCoreProfile)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined (__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
        //glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    }
    myOcctWindow = new GlfwOcctWindow(theWidth, theHeight, theTitle);
    glfwSetWindowUserPointer(myOcctWindow->getGlfwWindow(), this);

    // window callback
    glfwSetWindowSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onResizeCallback);
    glfwSetFramebufferSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onFBResizeCallback);

    // mouse callback
    glfwSetScrollCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseScrollCallback);
    glfwSetMouseButtonCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseButtonCallback);
    glfwSetCursorPosCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseMoveCallback);
}

// ================================================================
// Function : initViewer
// Purpose  :
// ================================================================
void GlfwOcctView::initViewer()
{
    if (myOcctWindow.IsNull()
        || myOcctWindow->getGlfwWindow() == nullptr)
    {
        return;
    }

    Handle(OpenGl_GraphicDriver) aGraphicDriver
        = new OpenGl_GraphicDriver(myOcctWindow->GetDisplay(), Standard_False);
    aGraphicDriver->SetBuffersNoSwap(Standard_True);

    Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
    aViewer->SetDefaultLights();
    aViewer->SetLightOn();
    aViewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
    aViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
    myView = aViewer->CreateView();
    //myView->SetImmediateUpdate(Standard_False);
    myView->SetWindow(myOcctWindow, myOcctWindow->NativeGlContext());
    myView->ChangeRenderingParams().ToShowStats = Standard_True;

    myContext = new AIS_InteractiveContext(aViewer);

    Handle(AIS_ViewCube) aCube = new AIS_ViewCube();
    aCube->SetSize(55);
    aCube->SetFontHeight(12);
    aCube->SetAxesLabels("", "", "");
    aCube->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_TriedronPers, Aspect_TOTP_LEFT_LOWER, Graphic3d_Vec2i(100, 100)));
    aCube->SetViewAnimation(this->ViewAnimation());
    aCube->SetFixedAnimationLoop(false);
    myContext->Display(aCube, false);
}

void GlfwOcctView::initGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& aIO = ImGui::GetIO();
    aIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(myOcctWindow->getGlfwWindow(), Standard_True);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup Dear ImGui style.
    //ImGui::StyleColorsClassic();
}

void GlfwOcctView::renderGui()
{
    ImGuiIO& aIO = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    // Hello IMGUI.
    ImGui::Begin("Hello");
    ImGui::Text("Hello ImGui!");
    ImGui::Text("Hello OpenCASCADE!");
    ImGui::Button("OK");
    ImGui::SameLine();
    ImGui::Button("Cancel");
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(myOcctWindow->getGlfwWindow());
}

// ================================================================
// Function : initDemoScene
// Purpose  :
// ================================================================
void GlfwOcctView::initDemoScene()
{
    if (myContext.IsNull())
    {
        return;
    }

    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);

    // gp_Ax2 anAxis;
    // anAxis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
    // Handle(AIS_Shape) aBox = new AIS_Shape(BRepPrimAPI_MakeBox(anAxis, 50, 50, 50).Shape());
    // myContext->Display(aBox, AIS_Shaded, 0, false);
    // anAxis.SetLocation(gp_Pnt(25.0, 125.0, 0.0));
    // Handle(AIS_Shape) aCone = new AIS_Shape(BRepPrimAPI_MakeCone(anAxis, 25, 0, 50).Shape());
    // myContext->Display(aCone, AIS_Shaded, 0, false);
    DoGeometryTest();
    TCollection_AsciiString aGlInfo;
    {
        TColStd_IndexedDataMapOfStringString aRendInfo;
        myView->DiagnosticInformation(aRendInfo, Graphic3d_DiagnosticInfo_Basic);
        for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aRendInfo); aValueIter.More(); aValueIter.Next())
        {
            if (!aGlInfo.IsEmpty()) { aGlInfo += "\n"; }
            aGlInfo += TCollection_AsciiString("  ") + aValueIter.Key() + ": " + aValueIter.Value();
        }
    }
    Message::DefaultMessenger()->Send(TCollection_AsciiString("OpenGL info:\n") + aGlInfo, Message_Info);
}

void GlfwOcctView::DoGeometryTest()
{
    gp_Pnt p1(0, 0, 0);
    gp_Pnt p2(10, 0, 0);
    gp_Pnt p3(10, 20, 0);
    gp_Pnt p4(0, 20, 0);
    ///定义四条直线,几何部分
    Handle(Geom_Curve) c1 = new Geom_Line(p1, gp_Dir(p2.XYZ() - p1.XYZ()));
    Handle(Geom_Curve) c2 = new Geom_Line(p2, gp_Dir(p3.XYZ() - p2.XYZ()));
    Handle(Geom_Curve) c3 = new Geom_Line(p3, gp_Dir(p4.XYZ() - p3.XYZ()));
    Handle(Geom_Curve) c4 = new Geom_Line(p4, gp_Dir(p1.XYZ() - p4.XYZ()));
    ///创建拓扑层
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(c1, p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(c2, p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(c3, p3, p4);

    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(c4, p4, p1);
    ///围起来
    BRepBuilderAPI_MakeWire wireBuilder;
    wireBuilder.Add(e1);
    wireBuilder.Add(e2);
    wireBuilder.Add(e3);
    wireBuilder.Add(e4);
    TopoDS_Wire wire = wireBuilder.Wire();
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire); ///注意，这里返回的是一个拓扑层的face，其引用了一个几何的surface，裁剪了几何的surface

    ///使用 BRep_Tool 拿 Surface
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
    ///强转类型
    Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surf); ///代表一个无穷大的平面
    auto pln = plane->Pln(); ///数学平面,获得原点和方向

    ///使用 BRep_Tool 拿 curve
    Standard_Real f, l;
    Handle(Geom_Curve) c3d = BRep_Tool::Curve(e1, f, l); ///3D空间，c(t)=(t,0,0),t~[f,l]

    Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(e1, face, f, l); ///UV空间，注意，它属于面，同一条边会存在于多个面内，u(t)=t,v(t)=0

    ///3D空间         c(t)=(x(t),y(t),z(t))
    ///UV空间         (u(y),v(t))
    ///Surface        r(u,v)
    ///因此，满足      c(t)=r(u(t),v(t))
    for (int i = 0; i <= 5; ++i)
    {
        double t = f + (l - f) * i / 5.0;

        gp_Pnt p3d = c3d->Value(t);

        gp_Pnt2d uv = c2d->Value(t);
        gp_Pnt pFromUV = surf->Value(uv.X(), uv.Y());

        // 比较 p3d 和 pFromUV
        int x = 1;
    }

    ///拉升face
    gp_Vec vec(0, 0, 10);
    TopoDS_Shape solid = BRepPrimAPI_MakePrism(face, vec);

    ///创建AIS_Shape
    Handle(AIS_Shape) aShape = new AIS_Shape(wire);
    myContext->Display(aShape, AIS_Shaded, 0, false);
}

// ================================================================
// Function : handleViewRedraw
// Purpose  :
// ================================================================
void GlfwOcctView::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                    const Handle(V3d_View)& theView)
{
  AIS_ViewController::handleViewRedraw(theCtx, theView);
  myToWaitEvents = !myToAskNextFrame;
}

// ================================================================
// Function : mainloop
// Purpose  :
// ================================================================
void GlfwOcctView::mainloop()
{
    while (!glfwWindowShouldClose(myOcctWindow->getGlfwWindow()))
    {
        // glfwPollEvents() for continuous rendering (immediate return if there are no new events)
        // and glfwWaitEvents() for rendering on demand (something actually happened in the viewer)
        if (myToWaitEvents)
        {
          glfwWaitEvents();
        }
        else
        {
          glfwPollEvents();
        }
        if (!myView.IsNull())
        {
            myView->InvalidateImmediate(); // redraw view even if it wasn't modified
            FlushViewEvents(myContext, myView, Standard_True);

            renderGui();
        }
    }
}

// ================================================================
// Function : cleanup
// Purpose  :
// ================================================================
void GlfwOcctView::cleanup()
{
    // Cleanup IMGUI.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (!myView.IsNull())
    {
        myView->Remove();
    }
    if (!myOcctWindow.IsNull())
    {
        myOcctWindow->Close();
    }

    glfwTerminate();
}

// ================================================================
// Function : onResize
// Purpose  :
// ================================================================
void GlfwOcctView::onResize(int theWidth, int theHeight)
{
    if (theWidth != 0
        && theHeight != 0
        && !myView.IsNull())
    {
        myView->Window()->DoResize();
        myView->MustBeResized();
        myView->Invalidate();
        FlushViewEvents(myContext, myView, true);
        renderGui();
    }
}

// ================================================================
// Function : onMouseScroll
// Purpose  :
// ================================================================
void GlfwOcctView::onMouseScroll(double theOffsetX, double theOffsetY)
{
    ImGuiIO& aIO = ImGui::GetIO();
    if (!myView.IsNull() && !aIO.WantCaptureMouse)
    {
        UpdateZoom(Aspect_ScrollDelta(myOcctWindow->CursorPosition(), int(theOffsetY * 8.0)));
    }
}

// ================================================================
// Function : onMouseButton
// Purpose  :
// ================================================================
void GlfwOcctView::onMouseButton(int theButton, int theAction, int theMods)
{
    ImGuiIO& aIO = ImGui::GetIO();
    if (myView.IsNull() || aIO.WantCaptureMouse)
    {
        return;
    }

    const Graphic3d_Vec2i aPos = myOcctWindow->CursorPosition();
    if (theAction == GLFW_PRESS)
    {
        PressMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
    else
    {
        ReleaseMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
}

// ================================================================
// Function : onMouseMove
// Purpose  :
// ================================================================
void GlfwOcctView::onMouseMove(int thePosX, int thePosY)
{
    if (myView.IsNull())
    {
        return;
    }

    ImGuiIO& aIO = ImGui::GetIO();
    if (aIO.WantCaptureMouse)
    {
        //myView->Redraw();
    }
    else
    {
        const Graphic3d_Vec2i aNewPos(thePosX, thePosY);
        UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), Standard_False);
    }
}

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
#include <BRepPrimAPI_MakeRevolution.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Line.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <IMeshTools_Parameters.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include  <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Graphic3d_Camera.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_Name.hxx>
#include <TPrsStd_AISPresentation.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

#include <iostream>

#include <GLFW/glfw3.h>

#include "SelectMgr.h"

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
    myContext->MainSelector()->AllowOverlapDetection(Standard_True);

    Handle(AIS_ViewCube) aCube = new AIS_ViewCube();
    aCube->SetSize(55);
    aCube->SetFontHeight(12);
    aCube->SetAxesLabels("", "", "");
    aCube->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_TriedronPers, Aspect_TOTP_LEFT_LOWER, Graphic3d_Vec2i(100, 100)));
    aCube->SetViewAnimation(this->ViewAnimation());
    aCube->SetFixedAnimationLoop(false);
    myContext->Display(aCube, false);

    SelectMgr::Instance().SetInteractiveContext(myContext);
    SelectMgr::Instance().SetViewer(aViewer);
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

    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
    Handle(AIS_Shape) aBox = new AIS_Shape(BRepPrimAPI_MakeBox(anAxis, 50, 50, 50).Shape());
    aBox->SetTransparency(0.6);
    SetShapeId("aBox",aBox);
    myContext->Display(aBox, AIS_Shaded, 0, false);
    anAxis.SetLocation(gp_Pnt(25.0, 125.0, 0.0));
    Handle(AIS_Shape) aCone = new AIS_Shape(BRepPrimAPI_MakeCone(anAxis, 25, 0, 50).Shape());
    SetShapeId("aCone",aCone);
    myContext->Display(aCone, AIS_WireFrame, 0, false); {
        // 参数说明：主半径（圆环中心到管子中心），管子半径
        Standard_Real majorRadius = 5.0; // 圆环主半径
        Standard_Real minorRadius = 3.0; // 圆环截面半径

        // 创建圆环
        TopoDS_Shape torus = BRepPrimAPI_MakeTorus(majorRadius, minorRadius).Shape();
        // 2. 配置离散参数（提高精度）
        IMeshTools_Parameters meshParams;
        meshParams.Deflection = minorRadius * 0.01; // 最大偏差（越小精度越高，建议 0.01~0.1）
        meshParams.Angle = 0.5; // 角度公差（弧度，越小三角形越多，建议 0.5~2.0）
        BRepMesh_IncrementalMesh mesher(torus, meshParams);

        // 3. 确保离散成功
        if (!mesher.IsDone()) {
            // 处理离散失败（如检查参数是否合理）
        }

        gp_Trsf mat;
        mat.SetTranslation(gp_Vec(100, 100, 100));
        torus.Move(mat);
        static bool isExport = true;
        if (!isExport) {
            ExportMeshData(torus, R""(C:\Users\ZQD\Desktop\data\torus.obj)"");
        }
        // 显示或进一步操作
        Handle(AIS_Shape) aisTorus = new AIS_Shape(torus);
        aisTorus->SetTransparency(0.3); // 可选：设置半透明
        SetShapeId("aisTorus",aisTorus);
        myContext->Display(aisTorus, AIS_Shaded, 0, false);
    } {
        auto origin = gp_Pnt{100, 0, 0};
        auto direction = gp_Dir{0, 0, 1};
        // 创建圆柱（箭头杆）
        gp_Ax2 shaftAxis(origin, direction); // 轴系：原点+方向

        BRepPrimAPI_MakeCylinder cylinderMaker(shaftAxis, 6 / 2, 100);
        if (!cylinderMaker.IsDone()) {
            int x = 1;
            //Standard_Failure::Raise("创建圆柱失败");
        }
        Handle(AIS_Shape) aRevolution = new AIS_Shape(cylinderMaker.Shape());
        aRevolution->SetTransparency(0.6);
        SetShapeId("aRevolution",aRevolution);
        myContext->Display(aRevolution, AIS_Shaded, 0, false);
    }
    //创建线段
    {
        // 定义线段的起点和终点
        gp_Pnt P1(0.0, 0.0, 0.0); // 起点 (x,y,z)
        gp_Pnt P2(-100.0, -50.0, 50.0); // 终点

        // 创建线段
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(P1, P2).Edge();

        // 可视化（可选）
        Handle(AIS_Shape) aisEdge = new AIS_Shape(edge);
        SetShapeId("aisEdge",aisEdge);
        myContext->Display(aisEdge, AIS_Shaded, 0, false);
    }

    CreateRandomModels(500,1000);

    TCollection_AsciiString aGlInfo; {
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

void GlfwOcctView::ExportMeshData(const TopoDS_Shape &shape, const std::string &filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件：" + filePath);
    }

    TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
    int vertexOffset = 0;

    while (faceExplorer.More()) {
        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
        TopLoc_Location loc; // 位置变换
        // 获取三角化网格（OCCT 7.6+ 最新API）
        const Handle(Poly_Triangulation) &tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) {
            faceExplorer.Next();
            continue;
        }

        // 1. 提取顶点（转换为全局坐标）
        const auto &verticesLocal = tri->InternalNodes();
        for (Standard_Integer i = verticesLocal.Lower(); i <= verticesLocal.Upper(); ++i) {
            gp_Pnt pntGlobal = verticesLocal.Value(i).Transformed(loc.Transformation()); // 关键：应用位置变换
            file << "v " << pntGlobal.X() << " " << pntGlobal.Y() << " " << pntGlobal.Z() << std::endl;
        }

        // 2. 提取三角形索引
        const Poly_Array1OfTriangle &triangles = tri->InternalTriangles();
        for (Standard_Integer i = triangles.Lower(); i <= triangles.Upper(); ++i) {
            Standard_Integer v1, v2, v3;
            triangles(i).Get(v1, v2, v3);
            // 索引需加上顶点偏移量（多面时累加）
            file << "f " << v1 + vertexOffset << " " << v2 + vertexOffset << " " << v3 + vertexOffset << std::endl;
        }

        vertexOffset += verticesLocal.Size();
        faceExplorer.Next();
    }

    file.close();
    std::cout << "全局网格数据已导出至：" << filePath << std::endl;
}

void GlfwOcctView::ComputeRayFromScreenPos(int x, int y, gp_Pnt &rayOrigin, gp_Dir &rayDir) const {
    // 获取视图和相机信息
    Handle(Graphic3d_Camera) camera = myView->Camera();
    Standard_Integer viewWidth,viewHeight;
    myView->Window()->Size(viewWidth,viewHeight);

    // 将屏幕坐标标准化到[-1, 1]范围
    Standard_Real nx = (2.0 * x) / viewWidth - 1.0;
    Standard_Real ny = 1.0 - (2.0 * y) / viewHeight; // Y轴反转，因为屏幕Y向下

    // 获取相机参数
    gp_Pnt eye = camera->Eye();
    gp_Pnt center = camera->Center();
    gp_Dir viewDir = (center.XYZ() - eye.XYZ()).Normalized();


    gp_Dir upDir = camera->Up();
    gp_Dir rightDir = viewDir.Crossed(upDir).XYZ().Normalized();

    // 计算视场角相关参数
    Standard_Real fovy = camera->FOVy();
    Standard_Real aspect = viewWidth / viewHeight;
    Standard_Real tanFovy = tan(fovy / 2.0);

    // 计算射线在相机坐标系中的方向
    gp_Dir localDir(
        nx * aspect * tanFovy,
        ny * tanFovy,
        1.0
    );

    // 转换到世界坐标系
    rayOrigin = eye;
    rayDir = gp_Dir(
        localDir.X() * rightDir.X() + localDir.Y() * upDir.X() + localDir.Z() * viewDir.X(),
        localDir.X() * rightDir.Y() + localDir.Y() * upDir.Y() + localDir.Z() * viewDir.Y(),
        localDir.X() * rightDir.Z() + localDir.Y() * upDir.Z() + localDir.Z() * viewDir.Z()
    );
}

void GlfwOcctView::SetShapeId(const std::string &Id, opencascade::handle<AIS_Shape> aisShape) {
    SelectMgr::Instance().SetShapeId(Id,aisShape);;
}

void GlfwOcctView::CreateRandomModels(Standard_Integer count, Standard_Real range) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disPos(-range/2, range/2);
    std::uniform_real_distribution<> disSize(10, 50);
    std::uniform_int_distribution<> disType(0, 2); // 0:立方体, 1:圆柱体, 2:球体

    for (Standard_Integer i = 0; i < count; ++i) {
        // 随机位置
        gp_Pnt pos(disPos(gen), disPos(gen), disPos(gen));

        // 随机尺寸
        Standard_Real size = disSize(gen);

        // 随机类型
        TopoDS_Shape shape;
        switch (disType(gen)) {
            case 0: // 立方体
                shape = BRepPrimAPI_MakeBox(pos, size, size, size).Shape();
                break;
            case 1: // 圆柱体
                shape = BRepPrimAPI_MakeCylinder(gp_Ax2(pos, gp_Dir(0, 0, 1)), size/2, size).Shape();
                break;
            case 2: // 球体
                shape = BRepPrimAPI_MakeSphere(pos, size/2).Shape();
                break;
        }

        // 创建并显示AIS对象
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        //aisShape->SetColor(Quantity_NOC_BLUE);
        myContext->Display(aisShape, AIS_Shaded, 0,false);
    }

    myView->FitAll();
    std::cout << "已创建 " << count << " 个随机模型" << std::endl;
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
        // gp_Pnt rayOrigin;
        // gp_Dir rayDirection;
        // ComputeRayFromScreenPos(aPos.x(),aPos.y(),rayOrigin,rayDirection);
        // SelectMgr::Instance().SetScreenMousePos(aPos.x(),aPos.y());
        // SelectMgr::Instance().GetHitPoint(rayOrigin, rayDirection);
        SelectMgr::Instance().Test(aPos.x(),aPos.y());
    }
    else {
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
        // gp_Pnt rayOrigin;
        // gp_Dir rayDirection;
        // ComputeRayFromScreenPos(aNewPos.x(),aNewPos.y(),rayOrigin,rayDirection);
        // SelectMgr::Instance().SetScreenMousePos(aNewPos.x(),aNewPos.y());
        // SelectMgr::Instance().GetHitPoint(rayOrigin, rayDirection);
        SelectMgr::Instance().TestForSnap(aNewPos.x(), aNewPos.y());
    }
}

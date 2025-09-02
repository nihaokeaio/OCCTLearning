//
// Created by ZQD on 25-8-11.
//

#include "SelectMgr.h"
#include <SelectMgr_SelectionManager.hxx>
#include <SelectMgr_ViewerSelector.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <V3d_Viewer.hxx>
#include <BVH_LinearBuilder.hxx>
#include <Select3D_SensitiveTriangle.hxx>
#include <SelectMgr_AxisIntersector.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <V3d_View.hxx>
#include <memory>

#include "SnapSystem.h"
#include "Timer.h"

SelectMgr &SelectMgr::Instance() {
    static SelectMgr instance;
    return instance;
}

void SelectMgr::InitSelectMgr() {
    mySelection = new SelectMgr_Selection();
}

auto SelectMgr::SetViewer(const opencascade::handle<V3d_Viewer>& viewer) -> void {
    m_Viewer = viewer;
    if (m_SnapSystem)
        m_SnapSystem->SetCurrentView(m_Viewer->ActiveViews().First());
}

auto SelectMgr::SetInteractiveContext(const opencascade::handle<AIS_InteractiveContext>& context) -> void {
    m_Context = context;
    m_SnapSystem = std::make_shared<SnapSystem>(m_Context);
    m_SnapSystem->EnableAllSnapType(true);
}

auto SelectMgr::GetIntersection(int screenX, int screenY, std::vector<IntersectionResult> &intersection) const {
    auto selectmgr = m_Context->SelectionManager();
    auto selector = selectmgr->Selector();
    selector->Pick(screenX,screenY, m_Viewer->ActiveViews().First());
    Intersection(intersection);
    //selector->ClearPicked();
    // auto selectorNb = selector->NbPicked();
    // for (Standard_Integer i = 1; i <= selectorNb; ++i) {
    //     auto data = selector->PickedData(i);
    //     auto owner = data.Entity->OwnerId();
    //     if (owner.IsNull())
    //         continue;
    //     Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(owner->Selectable());
    //     if (!shape.IsNull()) {
    //         IntersectionResult ints;
    //         ints.Object = shape;
    //         ints.Distance = data.Depth;
    //         ints.Point = data.Point;
    //         intersection.emplace_back(std::move(ints));
    //     }
    // }
}

auto SelectMgr::GetIntersection(const Ray &ray, std::vector<IntersectionResult> &intersection) const {
    auto selectmgr = m_Context->SelectionManager();
    auto selector = selectmgr->Selector();
    selector->Pick(gp_Ax1(ray.Origin,ray.Direction), m_Viewer->ActiveViews().First());
    Intersection(intersection);
    selector->ClearPicked();
    // auto selectorNb = selector->NbPicked();
    // for (Standard_Integer i = 1; i <= selectorNb; ++i) {
    //     auto data = selector->PickedData(i);
    //     auto owner = data.Entity->OwnerId();
    //     if (owner.IsNull())
    //         continue;
    //     Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(owner->Selectable());
    //     if (!shape.IsNull()) {
    //         IntersectionResult ints;
    //         ints.Object = shape;
    //         ints.Distance = data.Depth;
    //         ints.Point = data.Point;
    //         intersection.emplace_back(std::move(ints));
    //     }
    // }
}

auto SelectMgr::GetIntersection(std::vector<IntersectionResult> &intersection) const {
    Intersection(intersection);
}

auto SelectMgr::GetIntersectionUseBRepExtrema(const Ray& ray ,std::vector<IntersectionResult> &intersection) const {
    NCollection_List<Handle(SelectMgr_EntityOwner) > ownerList;
    m_Context->SelectionManager()->Selector()->ActiveOwners(ownerList);
    int count=0;
    auto objectIter=m_Context->ObjectIterator();
    for (; objectIter.More(); objectIter.Next()) {
        auto object = objectIter.Key();
        auto shape = Handle(AIS_Shape)::DownCast(object);
        if (!shape.IsNull()) {
            GetShapeIntersections(shape, gp_Lin(ray.Origin, ray.Direction), intersection);
        }
        ++count;
    }
    std::sort(intersection.begin(), intersection.end(), [](const IntersectionResult &a, const IntersectionResult &b) {
        return a.Distance < b.Distance;
    });
}


void SelectMgr::GetShapeIntersections(Handle(AIS_Shape) shape, const gp_Lin &ray,
    std::vector<IntersectionResult> &intersection) {
    // 将射线转换为无限细的边
    TopoDS_Edge rayEdge = BRepBuilderAPI_MakeEdge(ray).Edge();

    BRepExtrema_DistShapeShape distTool(shape->Shape(), rayEdge);
    if (!distTool.IsDone()) return;

    // 存储所有交点（自动排序）
   IntersectionResult ints;// <距离, 点>
    for (int i = 1; i <= distTool.NbSolution(); ++i) {
       auto  hit=distTool.PointOnShape1(i);
        ints.Point=hit;
        ints.Distance=hit.Distance(ray.Location());
        ints.Object=shape;
        intersection.emplace_back(std::move(ints));
    }
}

void SelectMgr::GetShapeIntersection(const Ray &ray,const opencascade::handle<AIS_InteractiveObject>& obj, std::vector<IntersectionResult> &intersection) const {
    if (!obj->IsKind(STANDARD_TYPE(AIS_Shape))) {
        return;  // 只处理几何形状
    }
    // 1. 获取模型的几何形状和变换
    Handle(AIS_Shape) shapeObj = Handle(AIS_Shape)::DownCast(obj);
    TopoDS_Shape shape = shapeObj->Shape();
    gp_Trsf transform = m_Context->Location(obj);  // 模型的位置变换

    // 2. 遍历模型中的所有面
    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    for (; faceExp.More(); faceExp.Next()) {
        const TopoDS_Face& face = TopoDS::Face(faceExp.Current());

        // 3. 检测射线与面的相交
        gp_Pnt hitPoint;
        if (GetFaceIntersection(ray, face, transform, hitPoint)) {
            // 计算距离（射线起点到交点）
            Standard_Real distance = ray.Origin.Distance(hitPoint);
            intersection.push_back({hitPoint, distance, obj});
        }
    }
}
bool SelectMgr::GetFaceIntersection(const Ray &ray, const TopoDS_Face &face, const gp_Trsf &transform,
                                    gp_Pnt &hitPoint) {
    // 1. 将射线转换到面的局部坐标系（考虑模型变换）
    gp_Trsf invTransform = transform.Inverted();  // 逆变换
    gp_Pnt localOrigin = ray.Origin.Transformed(invTransform);
    gp_Dir localDir = ray.Direction.Transformed(invTransform);

    // 2. 使用OCCT的求交工具检测射线与面的相交
    IntCurvesFace_Intersector intersector(face, 1e-5);

    // 执行求交：参数为（起点，方向，最小距离，最大距离）
    intersector.Perform(gp_Lin( localOrigin, localDir), 0.0, 1e6);

    if (intersector.IsDone() && intersector.NbPnt() > 0) {
        // 获取交点（转换回世界坐标系）
        hitPoint = intersector.Pnt(1);
        hitPoint.Transform(transform);
        return true;
    }
    return false;
}

void SelectMgr::Test(int mouseX,int mouseY) const {
    // 1. 初始化随机数引擎（使用真随机种子）
    std::random_device rd;  // 获取硬件熵（如果不可用，可能退化到伪随机）
    std::mt19937 gen(rd()); // 用 rd 的输出来初始化引擎
    // 2. 定义分布范围（例如 1~100 的整数）
    std::vector<IntersectionResult> intersection;
    std::uniform_int_distribution<int> dist(1, 500);
    double useTime = 0;
    for (int i=0;i<1;++i) {
        int screenX=dist(gen);
        int screenY=dist(gen);
        Timer t;
        GetIntersection(intersection);
        useTime+=t.elapsed();
    }
    std::cout << "耗时: " << useTime << " 毫秒" << std::endl;
    PrintResult(intersection);
    useTime = 0;
    intersection.clear();

    for (int i=0;i<0;++i) {
        int screenX=dist(gen);
        int screenY=dist(gen);
        gp_Ax1 ax1;
        GenerateRayFromMousePos(mouseX, mouseY, ax1);
        Ray ray;
        ray.Origin=ax1.Location();
        ray.Direction=ax1.Direction();
        Timer t;
        GetIntersection(ray,intersection);
        useTime+=t.elapsed();
    }
    std::cout << "耗时: " << useTime << " 毫秒" << std::endl;
    PrintResult(intersection);
    useTime = 0;
    intersection.clear();

    {
        Timer t;
        for (int i=0;i<1;++i) {
            int screenX=dist(gen);
            int screenY=dist(gen);
            GetIntersection(mouseX,mouseY,intersection);
        }
        useTime=t.elapsed();
    }
    std::cout << "耗时: " << useTime << " 毫秒" << std::endl;
    PrintResult(intersection);
    useTime = 0;
    intersection.clear();
}

void SelectMgr::TestForSnap(int mouseX, int mouseY) const {
    if (!m_SnapSystem)
        return;
    std::vector<IntersectionResult> intersections;
    GetIntersection(intersections);
    PrintResult(intersections);
    gp_Ax1 ray;
    GenerateRayFromMousePos(mouseX,mouseY,ray);
    gp_Pnt snapPoint;
    SnapTypes snapType;
    m_SnapSystem->SetMousePosition(gp_XY(mouseX, mouseY));
    {
        std::vector<IntersectionResult> intersections;
        GetIntersection(intersections);
        PrintResult(intersections);
    }
    if (!intersections.empty()) {
        auto ints = intersections.front();
        TopoDS_Shape shape=Handle(AIS_Shape)::DownCast(ints.Object)->Shape();
        gp_Pnt hitPoint=ints.Point;
        m_SnapSystem->Snap(Standard_True,shape,hitPoint,gp_Lin(ray),snapPoint,snapType);
    }
    else {
        TopoDS_Shape shape;
        m_SnapSystem->Snap(false,shape,gp_Pnt(),gp_Lin(ray),snapPoint,snapType);
    }
}

void SelectMgr::GenerateRayFromMousePos(const int mouseX, const int mouseY, gp_Ax1 &ray) const {
    // 获取视图和相机信息
    auto view= m_Viewer->ActiveViews().First();
    Handle(Graphic3d_Camera) camera = view->Camera();
    Standard_Integer viewWidth,viewHeight;
    view->Window()->Size(viewWidth,viewHeight);

    // 将屏幕坐标标准化到[-1, 1]范围
    Standard_Real nx = (2.0 * mouseX) / viewWidth - 1.0;
    Standard_Real ny = 1.0 - (2.0 * mouseY) / viewHeight; // Y轴反转，因为屏幕Y向下

    auto worldPoint= view->Camera()->UnProject(gp_Pnt(nx,ny,-1));
    {
        gp_Pnt cameraEye = camera->Eye();
        gp_Dir mouseDir=gp_Dir( worldPoint.XYZ()-cameraEye.XYZ());
        int x=1;
        ray.SetDirection(mouseDir);
        ray.SetLocation(cameraEye);
        return;
    }
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
    ray.SetLocation(eye);
    ray.SetDirection(gp_Dir(
        localDir.X() * rightDir.X() + localDir.Y() * upDir.X() + localDir.Z() * viewDir.X(),
        localDir.X() * rightDir.Y() + localDir.Y() * upDir.Y() + localDir.Z() * viewDir.Y(),
        localDir.X() * rightDir.Z() + localDir.Y() * upDir.Z() + localDir.Z() * viewDir.Z()
    ));
}

void SelectMgr::SetShapeId(std::string Id, opencascade::handle<AIS_Shape> shape) {
    myAisMap.insert({shape.get(),Id});
}

void SelectMgr::PrintResult(const std::vector<IntersectionResult> &intersection) const {
    return;
    if (intersection.empty()) {
        std::cout << "未与任何模型相交" << std::endl;
        m_Context->ClearSelected(Standard_True);  // 清除选择状态
    } else {
        IntersectionResult closest = intersection[0];
        std::string name="unName";
        auto iter=myAisMap.find(dynamic_cast<AIS_Shape*>(closest.Object.get()));
        if (iter!=myAisMap.end()) {
            name=iter->second;
        }
        std::cout << "最近交点: (" << closest.Point.X() << ", "
                  << closest.Point.Y() << ", " << closest.Point.Z() << ")" << std::endl;
        std::cout << "距离: " << closest.Distance << ", 对象: " << name << std::endl;
        // 高亮显示选中的对象
        m_Context->ClearSelected(Standard_False);
        m_Context->SetSelected(closest.Object, Standard_True);
    }
}

void SelectMgr::Intersection(std::vector<IntersectionResult> &intersection) const {
    auto selectmgr = m_Context->SelectionManager();
    auto selector = selectmgr->Selector();
    auto selectorNb = selector->NbPicked();
    for (Standard_Integer i = 1; i <= selectorNb; ++i) {
        auto data = selector->PickedData(i);
        auto owner = data.Entity->OwnerId();
        if (owner.IsNull())
            continue;
        Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(owner->Selectable());
        if (!shape.IsNull()) {
            IntersectionResult ints;
            ints.Object = shape;
            ints.Distance = data.Depth;
            ints.Point = data.Point;
            intersection.emplace_back(std::move(ints));
        }
    }
}

void SelectMgr::FindSnapPoints(const gp_Pnt &cursorPos, double radius) {

}





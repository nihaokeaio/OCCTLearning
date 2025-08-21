//
// Created by ZQD on 25-8-11.
//

#pragma once

#include <AIS_Shape.hxx>
#include <map>
#include <unordered_map>
#include <StdSelect_ViewerSelector3d.hxx>
#include <TopoDS_Face.hxx>


class SnapSystem;
class Select3D_SensitiveTriangulation;
class V3d_Viewer;
class AIS_InteractiveContext;

// 射线结构：起点和方向
struct Ray {
    gp_Pnt Origin;   // 射线起点（相机位置）
    gp_Dir Direction; // 射线方向
};

// 交点信息
struct IntersectionResult {
    gp_Pnt Point;               // 交点坐标
    Standard_Real Distance;     // 距离射线起点的距离
    Handle(AIS_InteractiveObject) Object; // 相交的对象
};

class SelectMgr {
public:
    static SelectMgr &Instance();

    void InitSelectMgr();

    void SetViewer(const opencascade::handle<V3d_Viewer> &viewer);

    void SetInteractiveContext(const opencascade::handle<AIS_InteractiveContext> &context);

    //屏幕像素为点击处与场景求交
    auto GetIntersection(int screenX,int screenY,std::vector<IntersectionResult>&intersection) const;

    //自定义射线方向与场景求交
    auto GetIntersection(const Ray& ray,std::vector<IntersectionResult>&intersection) const;

    //鼠标点击处的交点
    auto GetIntersection(std::vector<IntersectionResult>&intersection) const;

    //遍历所有AIS_Shape对象，对其求交
    auto GetIntersectionUseBRepExtrema(const Ray& ray ,std::vector<IntersectionResult>&intersection) const;

    //与AIS对象求交，使用BRepExtrema_DistShapeShape类求交
    static void GetShapeIntersections(Handle(AIS_Shape) shape, const gp_Lin& ray,std::vector<IntersectionResult>&intersection);

    //与AIS对象求交，与其面片求交
    void GetShapeIntersection(const Ray &ray, const opencascade::handle<AIS_InteractiveObject> &obj, std::vector<IntersectionResult> &intersection) const;
    static bool GetFaceIntersection(const Ray& ray, const TopoDS_Face& face, const gp_Trsf& transform, gp_Pnt& hitPoint);

    void Test(int mouseX,int mouseY) const;

    void TestForSnap(int mouseX,int mouseY) const;

    void GenerateRayFromMousePos(int mouseX,int mouseY,gp_Ax1& ray) const;

    void SetShapeId(std::string Id,Handle(AIS_Shape) shape);
public:
    void PrintResult(const std::vector<IntersectionResult>& intersection) const;

private:
    SelectMgr() = default;

    void Intersection(std::vector<IntersectionResult>&intersection) const;

    void FindSnapPoints(const gp_Pnt& cursorPos, double radius);


private:
    std::map<AIS_Shape, int> shapeToId;
    std::map<AIS_Shape, Handle(Select3D_SensitiveTriangulation)> sensitiveCache;
    Handle(SelectMgr_Selection) mySelection; // 选择集，管理敏感实体
    Handle(AIS_InteractiveContext) m_Context;
    Handle(V3d_Viewer) m_Viewer;
    int screenX;
    int screenY;
    std::map<AIS_Shape*,std::string> myAisMap;
    std::shared_ptr<SnapSystem> m_SnapSystem;
};

//
// Created by ZQD on 25-8-19.
//

#include "SnapSystem.h"

#include <AIS_Shape.hxx>

#include "FeatureExtractor.h"
#include <Bnd_Sphere.hxx>
#include <BRepBndLib.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Line.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

SnapSystem::SnapSystem(const opencascade::handle<AIS_InteractiveContext> &context): mySnapRadius(5),
                                                                                    myVisualizationEnabled(true) {
    myContext = context;
    // 初始化可视化元素
    Handle(Geom_Point) geomPoint=new Geom_CartesianPoint(gp_Pnt());
    mySnapMarker = new AIS_Point(geomPoint);
    mySnapMarker->SetColor(Quantity_NOC_RED);

    Handle(Geom_Line) geomLine=new Geom_Line(gp_Lin());
    mySnapLine = new AIS_Line(geomLine);
    mySnapLine->SetColor(Quantity_NOC_YELLOW);
    mySnapLine->SetWidth(1);
}

void SnapSystem::SetSnapRadius(Standard_Real radius) {
    mySnapRadius = radius;
}

void SnapSystem::EnableSnapType(SnapTypes type, Standard_Boolean enable) {
    if (enable)
    {
        if (!myEnabledTypes.Contains(type))
            myEnabledTypes.Add(type);
    }
    else
    {
        myEnabledTypes.Remove(type);
    }
}

void SnapSystem::EnableAllSnapType(Standard_Boolean enable) {
    EnableSnapType(SnapTypes::Vertex, enable);
    EnableSnapType(SnapTypes::EdgeEndpoint, enable);
    EnableSnapType(SnapTypes::EdgeMidpoint, enable);
    EnableSnapType(SnapTypes::OnEdge, enable);
    EnableSnapType(SnapTypes::FaceCenter, enable);
    EnableSnapType(SnapTypes::Custom, enable);
}

void SnapSystem::SetCurrentView(const Handle(V3d_View)& view)
{
    myCurrentView = view;
}

Standard_Boolean SnapSystem::Snap(Standard_Boolean hasHit, const TopoDS_Shape &hitShape, const gp_Pnt &hitPoint,
    const gp_Lin &pickRay, gp_Pnt &snapPoint, SnapTypes &snapType) {
    myFeatures.Clear();
    bool isSnap=SnapToDiscreteFeature(hasHit,hitShape,hitPoint,pickRay,snapPoint,snapType);
    if (isSnap)
        return true;

    isSnap=SnapToEdgeFeature(hasHit,hitShape,hitPoint,pickRay,snapPoint,snapType);
    if (!isSnap) {
       return false;
    }
    // 可视化反馈
    ClearVisualization();
    if (isSnap && myVisualizationEnabled)
    {
        VisualizeSnap(snapPoint, snapType);
    }

    return isSnap;
}

Standard_Boolean SnapSystem::SnapToDiscreteFeature(Standard_Boolean hasHit, const TopoDS_Shape &hitShape,
    const gp_Pnt &hitPoint, const gp_Lin &pickRay, gp_Pnt &snapPoint, SnapTypes &snapType) {
    myFeatures.Clear();
    // 根据是否有交点提取不同的特征
    if (hasHit && !hitShape.IsNull())
    {
        ExtractLocalFeatures(hitShape, hitPoint);
    }
    else
    {
        ExtractGlobalFeatures(pickRay);
    }
    // 查找最佳吸附点
    Standard_Boolean found = FindBestSnapPoint(hitPoint, pickRay, hasHit, snapPoint, snapType);

    // 可视化反馈
    ClearVisualization();
    if (found && myVisualizationEnabled)
    {
        VisualizeSnap(snapPoint, snapType);
    }

    return found;
}

Standard_Boolean SnapSystem::SnapToEdgeFeature(Standard_Boolean hasHit, const TopoDS_Shape &hitShape,
    const gp_Pnt &hitPoint, const gp_Lin &pickRay, gp_Pnt &snapPoint, SnapTypes &snapType) {

    myFeatures.Clear();
    // 根据是否有交点提取不同的特征
    if (hasHit && !hitShape.IsNull())
    {
        return SnapToLocalEdgeFeatures(hitShape,hitPoint,snapPoint,snapType);
    }
    else
    {
        return SnapToGlobalEdgeFeatures(pickRay,snapPoint,snapType);
    }
}

Standard_Boolean SnapSystem::SnapToLocalEdgeFeatures(const TopoDS_Shape &hitShape, const gp_Pnt &hitPoint,
    gp_Pnt &snapPoint, SnapTypes &snapType) {
    Standard_Real minDistance = mySnapRadius;
    Standard_Boolean found = Standard_False;

    // 遍历命中形状中的所有边
    TopExp_Explorer edgeExplorer(hitShape, TopAbs_EDGE);
    for (; edgeExplorer.More(); edgeExplorer.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());

        // 快速过滤：边的包围球与参考点的距离是否小于吸附半径
        Bnd_Box bnd;
        BRepBndLib::Add(edge, bnd);
        bnd.Enlarge(mySnapRadius/2);
        if (bnd.IsOut(hitPoint))
            continue;

        // 计算参考点到边的投影点
        gp_Pnt projPoint;
        Standard_Real distance;
        if (FeatureExtractor::ProjectPointOnEdge(hitPoint, edge, projPoint, distance)
            && distance < minDistance) {
            minDistance = distance;
            snapPoint = projPoint;
            snapType = SnapTypes::OnEdge;
            found = Standard_True;
            }
    }
    return found;
}

Standard_Boolean SnapSystem::SnapToGlobalEdgeFeatures(const gp_Lin& pickRay,
    gp_Pnt &snapPoint, SnapTypes &snapType) {

    Standard_Real minDistance = mySnapRadius;
    Standard_Boolean found = Standard_False;

    // 遍历所有可见且启用吸附的对象
    for (auto& aisObj : GetVisibleSnapObjects()) {
        const TopoDS_Shape& shape = GetShapeFromAIS(aisObj);

        Bnd_Box shapeBnd;
        BRepBndLib::Add(shape, shapeBnd);
        shapeBnd.Enlarge(mySnapRadius);
        if (shapeBnd.IsOut(pickRay))
            continue;

        // 遍历模型中的所有边
        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
        for (; edgeExplorer.More(); edgeExplorer.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());

            Bnd_Box edgeBnd;
            BRepBndLib::Add(shape, edgeBnd);
            edgeBnd.Enlarge(mySnapRadius/2);
            if (edgeBnd.IsOut(pickRay))
                continue;

            // 计算射线到边的最短距离点
            gp_Pnt projPoint;
            Standard_Real distance;
            if (FeatureExtractor::ProjectRayOnEdge(pickRay, edge, projPoint, distance)
                && distance < minDistance) {
                minDistance = distance;
                snapPoint = projPoint;
                snapType = SnapTypes::OnEdge;
                found = Standard_True;
                }
        }
    }
    return found;
}

void SnapSystem::ClearVisualization() {
    if (!myContext.IsNull())
    {
        myContext->Erase(mySnapMarker, true);
        myContext->Erase(mySnapLine, true);
        myContext->Remove(mySnapMarker, true);
        myContext->Remove(mySnapLine, true);
    }
}

void SnapSystem::SetVisualizationEnabled(Standard_Boolean enabled) {
    myVisualizationEnabled = enabled;
    if (!enabled)
    {
        ClearVisualization();
    }
}

void SnapSystem::ExtractLocalFeatures(const TopoDS_Shape &hitShape, const gp_Pnt &hitPoint) {
    // 提取命中形状的所有特征点
    FeatureExtractor::ExtractFeatures(hitShape, myFeatures, myEnabledTypes);

    // 过滤超出吸附半径的特征点
    NCollection_Sequence<FeaturePoint> filteredFeatures;
    for (const auto& feat : myFeatures)
    {
        if (feat.Point.Distance(hitPoint) <= mySnapRadius)
        {
            filteredFeatures.Append(feat);
        }
    }

    myFeatures = filteredFeatures;
}

void SnapSystem::ExtractGlobalFeatures(const gp_Lin &pickRay) {
    // 获取所有可见且启用吸附的对象
    auto visibleObjects = GetVisibleSnapObjects();

    for (const auto& aisObj : visibleObjects)
    {
        TopoDS_Shape shape = GetShapeFromAIS(aisObj);
        if (shape.IsNull())
            continue;

        // 计算形状的包围球
        Bnd_Box bnd;
        BRepBndLib::Add(shape, bnd);
        bnd.Enlarge(mySnapRadius);

        // 快速过滤：如果射线在包围盒外，则跳过
        bool isOutBnd= bnd.IsOut(pickRay);
        if (isOutBnd)
            continue;

        // 提取该形状的特征点
        NCollection_Sequence<FeaturePoint> shapeFeatures;
        FeatureExtractor::ExtractFeatures(shape, shapeFeatures, myEnabledTypes);

        // 过滤超出吸附半径的特征点
        for (const auto& feat : shapeFeatures)
        {
            if (DistanceToRay(feat.Point, pickRay) <= mySnapRadius)
            {
                myFeatures.Append(feat);
            }
        }
    }
}

Standard_Boolean SnapSystem::FindBestSnapPoint(const gp_Pnt &referencePoint, const gp_Lin &pickRay,
    Standard_Boolean isHit, gp_Pnt &bestPoint, SnapTypes &bestType) {
    if (myFeatures.IsEmpty())
        return Standard_False;

    Standard_Real minDistance = mySnapRadius + 1.0;
    Standard_Boolean found = Standard_False;

    // 根据是否有交点，使用不同的距离计算方式
    for (const auto& feat : myFeatures)
    {
        Standard_Real distance;

        if (isHit)
        {
            // 有交点，使用点到点的距离
            distance = referencePoint.Distance(feat.Point);
        }
        else
        {
            // 无交点，使用点到射线的距离
            distance = DistanceToRay(feat.Point, pickRay);
        }

        // 距离更小的点更优
        if (distance < minDistance)
        {
            minDistance = distance;
            bestPoint = feat.Point;
            bestType = feat.SnapType;
            found = Standard_True;
        }
        // 距离相同的情况下，按类型优先级排序
        else if (distance == minDistance)
        {
            // 简单的优先级判断：顶点 > 端点 > 中点 > 边上点 > 面中心
            auto GetPriority = [](const SnapTypes& type) {
                switch (type)
                {
                    case SnapTypes::Vertex: return 5;
                    case SnapTypes::EdgeEndpoint: return 4;
                    case SnapTypes::EdgeMidpoint: return 3;
                    case SnapTypes::OnEdge: return 2;
                    case SnapTypes::FaceCenter: return 1;
                    default: return 0;
                }
            };

            if (GetPriority(feat.SnapType) > GetPriority(bestType))
            {
                bestPoint = feat.Point;
                bestType = feat.SnapType;
            }
        }
    }

    return found;
}

Standard_Real SnapSystem::DistanceToRay(const gp_Pnt &point, const gp_Lin &ray) {
    // 计算点到射线的最短距离
    gp_Vec v(ray.Location(), point);
    gp_Vec dir(ray.Direction());
    Standard_Real len = dir.Magnitude();

    if (len < Precision::Confusion())
        return v.Magnitude();

    dir.Normalize();
    Standard_Real t = v * dir;

    if (t < 0.0)
        return v.Magnitude();

    gp_Vec proj = dir * t;
    return (v - proj).Magnitude();
}

NCollection_Sequence<opencascade::handle<AIS_InteractiveObject>> SnapSystem::GetVisibleSnapObjects() {
    NCollection_Sequence<Handle(AIS_InteractiveObject)> result;

    if (myContext.IsNull())
        return result;

    AIS_ListOfInteractive intObjects;
    myContext->ObjectsInside(intObjects,AIS_KindOfInteractive_Shape);
    // 遍历交互上下文中的所有对象
    for (const auto& obj : intObjects) {
        if (myContext->IsDisplayed(obj) && IsObjectSnapEnabled(obj))
        {
            result.Append(obj);
        }
    }
    return result;
}

Standard_Boolean SnapSystem::IsObjectSnapEnabled(const opencascade::handle<AIS_InteractiveObject> &obj) {
    return  Standard_True;
}

void SnapSystem::VisualizeSnap(const gp_Pnt &snapPoint, SnapTypes type) {
    if (myContext.IsNull() || myCurrentView.IsNull())
        return;

    // 设置标记点
    Handle(Geom_CartesianPoint) gemoPoint= new Geom_CartesianPoint(snapPoint);
    mySnapMarker->SetComponent(gemoPoint);

    // 根据吸附类型设置不同颜色
    switch (type)
    {
        case SnapTypes::Vertex:
        case SnapTypes::EdgeEndpoint:
            mySnapMarker->SetColor(Quantity_NOC_RED);
            break;
        case SnapTypes::EdgeMidpoint:
            mySnapMarker->SetColor(Quantity_NOC_GREEN);
            break;
        case SnapTypes::OnEdge:
            mySnapMarker->SetColor(Quantity_NOC_BLUE);
            break;
        case SnapTypes::FaceCenter:
            mySnapMarker->SetColor(Quantity_NOC_YELLOW);
            break;
        default:
            mySnapMarker->SetColor(Quantity_NOC_RED);
    }

    myContext->Display(mySnapMarker, Standard_True);
    //myCurrentView->Redraw();
}

TopoDS_Shape SnapSystem::GetShapeFromAIS(const opencascade::handle<AIS_InteractiveObject> &aisObj) {
    // 从AIS对象获取拓扑形状
    Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(aisObj);
    if (!aisShape.IsNull())
    {
        return aisShape->Shape();
    }
    return TopoDS_Shape();
}


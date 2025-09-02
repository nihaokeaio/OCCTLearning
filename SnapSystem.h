//
// Created by ZQD on 25-8-19.
//

#ifndef SNAPSYSTEM_H
#define SNAPSYSTEM_H
#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Handle.hxx>

#include "SnapTypes.h"


class SnapSystem {
public:
    SnapSystem(const Handle(AIS_InteractiveContext)& context);

    // 设置吸附半径（世界坐标系）
    void SetSnapRadius(Standard_Real radius);

    // 启用/禁用特定吸附类型
    void EnableSnapType(SnapTypes type, Standard_Boolean enable);
    void EnableAllSnapType(Standard_Boolean enable);

    // 设置当前视图（用于坐标转换）
    void SetCurrentView(const Handle(V3d_View)& view);

    void SetMousePosition(gp_XY mousePos);
    // 执行吸附操作
    // hasHit: 是否有射线与物体的交点
    // hitShape: 命中的形状（hasHit为true时有效）
    // hitPoint: 射线交点（hasHit为true时有效）
    // pickRay: 拾取射线
    // 返回值：是否成功吸附
    Standard_Boolean Snap(Standard_Boolean hasHit,
                         const TopoDS_Shape& hitShape,
                         const gp_Pnt& hitPoint,
                         const gp_Lin& pickRay,
                         gp_Pnt& snapPoint,
                         SnapTypes& snapType);

    Standard_Boolean SnapToDiscreteFeature(Standard_Boolean hasHit,
                         const TopoDS_Shape& hitShape,
                         const gp_Pnt& hitPoint,
                         const gp_Lin& pickRay,
                         gp_Pnt& snapPoint,
                         SnapTypes& snapType);

    Standard_Boolean SnapToEdgeFeature(Standard_Boolean hasHit,
                         const TopoDS_Shape& hitShape,
                         const gp_Pnt& hitPoint,
                         const gp_Lin& pickRay,
                         gp_Pnt& snapPoint,
                         SnapTypes& snapType);

    Standard_Boolean SnapToLocalEdgeFeatures(const TopoDS_Shape& hitShape,
                                                     const gp_Pnt& hitPoint,
                                                     gp_Pnt& snapPoint,
                                                     SnapTypes& snapType) ;

    Standard_Boolean SnapToGlobalEdgeFeatures( const gp_Lin& pickRay,
                                                     gp_Pnt& snapPoint,
                                                     SnapTypes& snapType) ;
    // 清除所有临时可视化元素
    void ClearVisualization();

    // 设置是否显示吸附反馈
    void SetVisualizationEnabled(Standard_Boolean enabled);

private:
    // 从命中的形状提取局部特征（有交点情况）
    void ExtractLocalFeatures(const TopoDS_Shape& hitShape, const gp_Pnt& hitPoint);

    // 从所有可见物体提取全局特征（无交点情况）
    void ExtractGlobalFeatures(const gp_Lin& pickRay);

    // 筛选最佳吸附点
    Standard_Boolean FindBestSnapPoint(const gp_Pnt& referencePoint,
                                      const gp_Lin& pickRay,
                                      Standard_Boolean isHit,
                                      gp_Pnt& bestPoint,
                                      SnapTypes& bestType);

    // 计算点到射线的最短距离
    static Standard_Real DistanceToRay(const gp_Pnt& point, const gp_Lin& ray);

    // 获取所有可见且启用吸附的对象
    NCollection_Sequence<Handle(AIS_InteractiveObject)> GetVisibleSnapObjects();

    // 检查对象是否启用吸附
    static Standard_Boolean IsObjectSnapEnabled(const Handle(AIS_InteractiveObject)& obj);

    // 绘制吸附反馈
    void VisualizeSnap(const gp_Pnt& snapPoint, SnapTypes type);

    void VisualizeSnapRect();
    // 从AIS对象获取拓扑形状
    static TopoDS_Shape GetShapeFromAIS(const Handle(AIS_InteractiveObject)& aisObj);

private:
    Handle(AIS_InteractiveContext) myContext;  // 交互上下文
    Handle(V3d_View) myCurrentView;            // 当前视图
    Standard_Real mySnapRadius;                // 吸附半径
    NCollection_Map<SnapTypes> myEnabledTypes;  // 启用的吸附类型
    NCollection_Sequence<FeaturePoint> myFeatures;  // 特征点集合

    // 可视化相关
    Standard_Boolean myVisualizationEnabled;
    Handle(AIS_Point) mySnapMarker;
    Handle(AIS_Line) mySnapLine;
    Handle(AIS_Shape) myToleranceShape;
    gp_XY myMousePosition;
    int snapToleranceRange;
};



#endif //SNAPSYSTEM_H

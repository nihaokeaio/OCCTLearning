//
// Created by ZQD on 25-8-19.
//

#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <gp_Pnt.hxx>
#include <gp_Lin.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <NCollection_Sequence.hxx>
#include <NCollection_Map.hxx>
#include <V3d_View.hxx>
#include <AIS_Point.hxx>
#include <AIS_Line.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include "SnapTypes.h"

class FeatureExtractor {
public:
    // 从形状中提取所有特征点
    static void ExtractFeatures(const TopoDS_Shape& shape,
                               NCollection_Sequence<FeaturePoint>& features,
                               const NCollection_Map<SnapTypes>& enabledTypes);

    // 从顶点提取特征
    static void ExtractVertexFeatures(const TopoDS_Vertex& vertex,
                                     NCollection_Sequence<FeaturePoint>& features);

    // 从边提取特征
    static void ExtractEdgeFeatures(const TopoDS_Edge& edge,
                                   NCollection_Sequence<FeaturePoint>& features,
                                   const NCollection_Map<SnapTypes>& enabledTypes);

    // 从面提取特征
    static void ExtractFaceFeatures(const TopoDS_Face& face,
                                   NCollection_Sequence<FeaturePoint>& features,
                                   const NCollection_Map<SnapTypes>& enabledTypes);

    // 检查点是否在边上
    static Standard_Boolean IsPointOnEdge(const gp_Pnt& point, const TopoDS_Edge& edge, Standard_Real tolerance = 1e-5);

    // 计算点到边的投影
    static Standard_Boolean ProjectPointOnEdge(const gp_Pnt& point, const TopoDS_Edge& edge, gp_Pnt& projection, Standard_Real &distance);

    static Standard_Boolean ProjectRayOnEdge(const gp_Lin& ray,
                                            const TopoDS_Edge& edge,
                                            gp_Pnt& projPoint,
                                            Standard_Real& distance);
};



#endif //FEATUREEXTRACTOR_H

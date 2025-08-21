//
// Created by ZQD on 25-8-19.
//

#include "FeatureExtractor.h"

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>

void FeatureExtractor::ExtractFeatures(const TopoDS_Shape &shape, NCollection_Sequence<FeaturePoint> &features,
                                       const NCollection_Map<SnapTypes> &enabledTypes) {
    // 提取顶点特征
    if (enabledTypes.Contains(SnapTypes::Vertex))
    {
        TopExp_Explorer exp(shape, TopAbs_VERTEX);
        for (; exp.More(); exp.Next())
        {
            TopoDS_Vertex vertex = TopoDS::Vertex(exp.Current());
            ExtractVertexFeatures(vertex, features);
        }
    }

    // 提取边特征
    if (enabledTypes.Contains(SnapTypes::EdgeEndpoint) ||
        enabledTypes.Contains(SnapTypes::EdgeMidpoint) ||
        enabledTypes.Contains(SnapTypes::OnEdge))
    {
        TopExp_Explorer exp(shape, TopAbs_EDGE);
        for (; exp.More(); exp.Next())
        {
            TopoDS_Edge edge = TopoDS::Edge(exp.Current());
            ExtractEdgeFeatures(edge, features, enabledTypes);
        }
    }

    // 提取面特征
    if (enabledTypes.Contains(SnapTypes::FaceCenter))
    {
        TopExp_Explorer exp(shape, TopAbs_FACE);
        for (; exp.More(); exp.Next())
        {
            TopoDS_Face face = TopoDS::Face(exp.Current());
            ExtractFaceFeatures(face, features, enabledTypes);
        }
    }
}

void FeatureExtractor::ExtractVertexFeatures(const TopoDS_Vertex &vertex, NCollection_Sequence<FeaturePoint> &features) {
    gp_Pnt pnt = BRep_Tool::Pnt(vertex);
    features.Append(FeaturePoint{pnt, SnapTypes::Vertex, vertex});
}

void FeatureExtractor::ExtractEdgeFeatures(const TopoDS_Edge &edge, NCollection_Sequence<FeaturePoint> &features,
    const NCollection_Map<SnapTypes> &enabledTypes) {
    // 获取边的曲线
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge,first,last);
    if (curve.IsNull())
        return;


    //curve->Range(first, last);

    // 提取端点
    if (enabledTypes.Contains(SnapTypes::EdgeEndpoint))
    {
        gp_Pnt startPnt = curve->Value(first);
        features.Append(FeaturePoint{startPnt, SnapTypes::EdgeEndpoint, edge});

        gp_Pnt endPnt = curve->Value(last);
        features.Append(FeaturePoint{endPnt, SnapTypes::EdgeEndpoint, edge});
    }

    // 提取中点
    if (enabledTypes.Contains(SnapTypes::EdgeMidpoint))
    {
        gp_Pnt midPnt = curve->Value((first + last) / 2.0);
        features.Append(FeaturePoint{midPnt, SnapTypes::EdgeMidpoint, edge});
    }
}

void FeatureExtractor::ExtractFaceFeatures(const TopoDS_Face &face, NCollection_Sequence<FeaturePoint> &features,
    const NCollection_Map<SnapTypes> &enabledTypes) {
    // 计算面的中心（简化处理，取边界框中心）
    Bnd_Box bnd;
    BRepBndLib::Add(face, bnd);

    gp_Pnt center= (bnd.CornerMax().XYZ()+bnd.CornerMin().XYZ())/2.0;

    features.Append(FeaturePoint{center, SnapTypes::FaceCenter, face});
}

Standard_Boolean FeatureExtractor::
IsPointOnEdge(const gp_Pnt &point, const TopoDS_Edge &edge, Standard_Real tolerance) {
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge,first, last);
    if (curve.IsNull())
        return Standard_False;

    GeomAPI_ProjectPointOnCurve proj(point, curve);
    return proj.LowerDistance() <= tolerance;
}

Standard_Boolean FeatureExtractor::
ProjectPointOnEdge(const gp_Pnt &point, const TopoDS_Edge &edge, gp_Pnt &projection, Standard_Real &distance) {
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge,first, last);
    if (curve.IsNull())
        return Standard_False;

    GeomAPI_ProjectPointOnCurve proj(point, curve);
    if (proj.NbPoints()>0)
    {
        projection = proj.NearestPoint();
        distance=proj.LowerDistance();
        return Standard_True;
    }
    return Standard_False;
}

Standard_Boolean FeatureExtractor::ProjectRayOnEdge(const gp_Lin &ray, const TopoDS_Edge &edge, gp_Pnt &projPoint,
    Standard_Real &distance) {
    // 获取线段的几何曲线和参数范围
    Standard_Real edgeFirst, edgeLast;
    Handle(Geom_Curve) edgeCurve = BRep_Tool::Curve(edge,edgeFirst, edgeLast);
    if (edgeCurve.IsNull())
        return Standard_False;

    // 计算射线与曲线的最短距离（使用OCCT的几何工具）
    // 将射线转换为无限细的边
    TopoDS_Edge rayEdge = BRepBuilderAPI_MakeEdge(ray).Edge();
    Standard_Real rayFirst, rayLast;
    Handle(Geom_Curve) rayCurve = BRep_Tool::Curve(rayEdge,rayFirst, rayLast);
    if (rayCurve.IsNull())
        return Standard_False;

    GeomAPI_ExtremaCurveCurve distanceTool(rayCurve, edgeCurve,rayFirst, rayLast,edgeFirst,edgeLast);
    if (distanceTool.NbExtrema()>0) {
        gp_Pnt pointFirst, pointLast;
        distanceTool.NearestPoints(pointFirst, pointLast);
        projPoint = pointLast;
        distance=distanceTool.LowerDistance();
        return Standard_True;
    }
    return  Standard_False;
}

//
// Created by ZQD on 25-8-19.
//

#ifndef SNAPTYPES_H
#define SNAPTYPES_H
#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>


enum class SnapTypes {
    None,
    Vertex,
    EdgeEndpoint,   // 边端点
    EdgeMidpoint,   // 边中点
    OnEdge,         // 边上任意点
    FaceCenter,
    Custom
};

struct FeaturePoint {
    gp_Pnt Point;
    SnapTypes SnapType;
    TopoDS_Shape SourceShape;
    int Priority;
};

struct SnapResult {
    gp_Pnt Location;
    SnapTypes SnapType;
    TopoDS_Shape SourceShape;
    bool isVaild;
};


#endif //SNAPTYPES_H

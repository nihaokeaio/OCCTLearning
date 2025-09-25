//
// Created by gmh on 24-9-20.
//

#ifndef ROSEFINCH_POI_INTERSECT_CONTEXT_H
#define ROSEFINCH_POI_INTERSECT_CONTEXT_H

#include "bvh/v2/vec.h"
#include "SnapCaptureType_Semantic.h"
#include "V3d_View.hxx"



template<int Demension>
class POI_Intersect_Context {
  using Vec = ::bvh::v2::Vec<double, Demension>;

 public:
  POI_Intersect_Context() = default;
  virtual ~POI_Intersect_Context() = default;
  // delete copy constructor
  POI_Intersect_Context(const POI_Intersect_Context &) = delete;

  // 锚点，用于二元约束关系和正交约束
  Vec Anchor = Vec(std::numeric_limits<double>::max());
  // 捕捉类型
  SnapCaptureType_Semantic CaptureType;

  // 参考平面
  Vec ReferencePlaneOri = Vec(0);
  Vec ReferencePlaneXAxis = Vec(0);
  Vec ReferencePlaneNormal = Vec(0);

  // occt 视图
  Handle(V3d_View) OCC_View = nullptr;
};



#endif//ROSEFINCH_POI_INTERSECT_CONTEXT_H

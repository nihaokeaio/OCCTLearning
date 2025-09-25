//
// Created by gmh on 24-9-20.
//

#ifndef ROSEFINCH_SNAPCAPTURETYPE_SEMANTIC_H
#define ROSEFINCH_SNAPCAPTURETYPE_SEMANTIC_H

#include <string>
#include <vector>

#include "Data_Semantic.h"



class SnapCaptureType_Semantic : public Data_Semantic<SnapCaptureType_Semantic>{
 public:
  inline static const std::string ForbidCapture = "ForbidCapture"; // 停用捕捉

  /*
   * 一元关系
   * */
  // 最近点
  inline static const std::string MostClosedPoint = "MostClosedPoint";
  // 孤立点
  inline static const std::string IsolatedPoint = "IsolatedPoint";
  // 端点
  inline static const std::string EndingPoint = "EndingPoint";
  // 中点
  inline static const std::string MidPoint = "MidPoint";
  /*
   * AB二元关系，（A:anchor_point 和 poi 交点形成的直线. B: poi本身的几何要素.）
   * */
  // 垂足
  inline static const std::string FootPoint = "FootPoint";
  // 切点
  inline static const std::string TangentPoint = "TangentPoint";
  /*
   * 多元关系，多个poi之间的关系
   * */
  // 交点
  inline static const std::string IntersectionPoint = "IntersectionPoint";
  // 视线交点
  inline static const std::string ViewIntersectionPoint = "ViewIntersectionPoint";


  inline static const std::vector<std::string_view> Views = {
      std::string_view(ForbidCapture),
      std::string_view(MostClosedPoint),
      std::string_view(IsolatedPoint),
      std::string_view(EndingPoint),
      std::string_view(MidPoint),
      std::string_view(FootPoint),
      std::string_view(TangentPoint),
      std::string_view(IntersectionPoint),
      std::string_view(ViewIntersectionPoint)
  };

 public:
  SnapCaptureType_Semantic() {};
  DECLARE_SEMANTIC_SUCCESSOR_PARAM_CONSTRUCTOR(SnapCaptureType_Semantic)
};



#endif//ROSEFINCH_SNAPCAPTURETYPE_SEMANTIC_H

//
// Created by gmh on 24-8-16.
//

#ifndef ROSEFINCH_POI_DATA_H
#define ROSEFINCH_POI_DATA_H

#include "bvh/v2/bbox.h"
#include "bvh/v2/ray.h"
#include "POI_Data_Semantic.h"
#include "POI_Intersect_Context.h"



/*
  Graph drawing by ascii flow:
  Class Diagram:

 ┌────────────────────────────────────────────────────────────────┐
 │    POI_Data <int Dimension, class DataSource, class Derived>   │
 └──┬─────────────────────────────────────────────────────────────┘
    │    ┌──────────────────────────────────────────────────┐
    ┼────►  POI_Data_Line <int Dimension, class DataSource> │
    │    └──────────────────────────────────────────────────┘
    │    ┌──────────────────────────────────────────────────┐
    ┼────► POI_Data_Point <int Dimension, class DataSource> │
    │    └──────────────────────────────────────────────────┘
    │    ┌────────────────────────────────────────────────────┐
    ├────► POI_Data_Segment <int Dimension, class DataSource> │
    │    └────────────────────────────────────────────────────┘
    │    ┌──────────────────────┐
    └────► UnImpl...            │
         │ POI_Data_NurbsCurve  │
         │ and so on ...        │
         └──────────────────────┘

  POI_Data 负责提供静态接口，用于获取 POI 的 AABB 包围盒，以及与 Ray 或者 Point 的交点计算 Intersect()。

 * */

class POI_Data_Base {
public:
    virtual ~POI_Data_Base()=default;
};


template<int Dimension, class DataSource, class Derived>
class POI_Data : public POI_Data_Base{
  using Scalar = double;
  using BBox = ::bvh::v2::BBox<Scalar, Dimension>;
  using Vec = ::bvh::v2::Vec<Scalar, Dimension>;
  using Ray = ::bvh::v2::Ray<Scalar, Dimension>;

 public:
  virtual ~POI_Data() = default;

  int priority = 0;
  POI_Data_Semantic semantic;

 private:
  DataSource m_DataSource;
 public:
  const DataSource & GetDataSource() const {
    return m_DataSource;
  }
  void SetDataSource(const DataSource &dataSource) {
      m_DataSource = dataSource;
  }

 public:
  BBox AABB() {
    return static_cast<Derived *>(this)->GetAABB();
  }

  // Returns a tuple of bool, Vec, Vec.
  // Bool is true if the ray intersects the object, false otherwise.
  // If the ray intersects the object, the first Vec is the intersection point on query,
  // and the second Vec is the intersection point on the POI data.
  // int is the priority of POI data
  // POI_Data_Semantic is the POI Semantic
  // const DataSource & is the data source define by user.
  // SnapCaptureType_Semantic is the semantic of the snapping capture type, like FootPoint.

  typedef std::tuple<bool, Vec, Vec, int, POI_Data_Semantic, const DataSource &, std::string> POI_Int_Res;

  POI_Int_Res Inersect(const Ray &ray, const POI_Intersect_Context<Dimension> &context) {
    return static_cast<Derived *>(this)->IntersectByRay(ray, context);
  }

  POI_Int_Res Intersect(const Vec &point, const POI_Intersect_Context<Dimension> &context) {
    return static_cast<Derived *>(this)->IntersectByPoint(point, context);
  }
};


#endif//ROSEFINCH_POI_DATA_H

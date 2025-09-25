//
// Created by gmh on 24-8-16.
//

#ifndef ROSEFINCH_POI_DATA_POINT_H
#define ROSEFINCH_POI_DATA_POINT_H

#include "POI_Data.h"



template<int Dimension, class DataSource>
class POI_Data_Point : public POI_Data<Dimension,
                                       DataSource,
                                       POI_Data_Point<Dimension, DataSource>>{
 public:
  using BaseType = POI_Data<Dimension, DataSource, POI_Data_Point<Dimension, DataSource>>;
 private:
  using Scalar = double;
  using BBox = ::bvh::v2::BBox<Scalar, Dimension>;
  using Vec = ::bvh::v2::Vec<Scalar, Dimension>;
  using Ray = ::bvh::v2::Ray<Scalar, Dimension>;

 public:
  Vec m_Point;
 private:
  Scalar torlerance = 30;
 public:
  POI_Data_Point(const Vec &point) : m_Point(point) {}
  POI_Data_Point(const Vec &point, double torlerance) : m_Point(point), torlerance(torlerance) {}

  BBox GetAABB() const {
    Scalar fix_tor = torlerance;
    if (fix_tor < 0) {
      fix_tor = 50;
    }
    Vec min = m_Point - Vec(fix_tor);
    Vec max = m_Point + Vec(fix_tor);
    return BBox(min, max);
  }

  typename BaseType::POI_Int_Res IntersectByRay(const Ray &ray, const POI_Intersect_Context<Dimension> &context) {
    if (context.CaptureType.Contain(SnapCaptureType_Semantic::ForbidCapture) ||
        ! context.CaptureType.Contain(SnapCaptureType_Semantic::IsolatedPoint)) {
      return {
          false, Vec(0), m_Point, BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }
    Vec intersection_point;
    Vec intersection_point_on_query;
    auto normal_ray = Ray(ray.org, ::bvh::v2::normalize(ray.dir));

    Vec org_to_point = m_Point - ray.org;
    Vec dir = ::bvh::v2::normalize(ray.dir);
    if (::bvh::v2::dot(org_to_point, dir) < 0) {
      return {false,
             intersection_point_on_query,
             m_Point,
             BaseType::priority,
             BaseType::semantic,
             BaseType::GetDataSource(),
             {}};
    }
    Vec intersection_point_on_line = ray.org + ::bvh::v2::dot(org_to_point, dir) * dir;

    Scalar distance = ::bvh::v2::length(intersection_point_on_line - m_Point);
    if (distance < torlerance || torlerance < 0) {
      return {
          true,
          intersection_point_on_line,
          m_Point,
          BaseType::priority,
          BaseType::semantic,
          BaseType::GetDataSource(),
          SnapCaptureType_Semantic::IsolatedPoint};
    } else {
      return {
          false, intersection_point_on_line, m_Point, BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }
  }

  typename BaseType::POI_Int_Res IntersectByPoint(const Vec &point, const POI_Intersect_Context<Dimension> &context) {
    if (context.CaptureType.Contain(SnapCaptureType_Semantic::ForbidCapture)
        || !context.CaptureType.Contain(SnapCaptureType_Semantic::IsolatedPoint)) {
      return {false, Vec(0), m_Point, BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }
    Vec org_to_point = point - m_Point;
    Scalar distance = ::bvh::v2::length(org_to_point);
    if (distance < torlerance || torlerance < 0) {
      return {
          true,
          point,
          m_Point,
          BaseType::priority,
          BaseType::semantic,
          BaseType::GetDataSource(),
          SnapCaptureType_Semantic::IsolatedPoint};
    } else {
      return {false, point, m_Point, BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), SnapCaptureType_Semantic::IsolatedPoint};
    }
  }

};



#endif//ROSEFINCH_POI_DATA_POINT_H

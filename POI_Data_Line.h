//
// Created by gmh on 24-8-16.
//

#ifndef ROSEFINCH_POI_DATA_LINE_H
#define ROSEFINCH_POI_DATA_LINE_H

#include "POI_Data.h"

namespace {
using Scalar = double;

}// namespace


template<int Dimension, class DataSource>
class POI_Data_Line : public POI_Data<Dimension,
                                      DataSource,
                                      POI_Data_Line<Dimension, DataSource>> {
 public:
  using BaseType = POI_Data<Dimension, DataSource, POI_Data_Line<Dimension, DataSource>>;
 private:
  using Scalar = double;
  using BBox = ::bvh::v2::BBox<Scalar, Dimension>;
  using Vec = ::bvh::v2::Vec<Scalar, Dimension>;
  using Ray = ::bvh::v2::Ray<Scalar, Dimension>;

 public:
  Ray m_line;
 private:
  Scalar torlerance = 10;

 public:
  POI_Data_Line(const Vec &org, const Vec &dir) : m_line(org, ::bvh::v2::normalize(dir)) {}
  POI_Data_Line(const Vec &org, const Vec &dir, double torlerance) : m_line(org, ::bvh::v2::normalize(dir)),
                                                                     torlerance(torlerance) {}

  BBox GetAABB() const {
    BBox aabb = BBox::make_empty();
    return aabb;
  }

  typename BaseType::POI_Int_Res IntersectByRay(const Ray &ray, const POI_Intersect_Context<Dimension> &context) {
    if (context.CaptureType.Contain(SnapCaptureType_Semantic::ForbidCapture)) {
      return {
          false, Vec(0), Vec(0), BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }

    Vec intersection_point;
    Vec intersection_point_on_query;
    Scalar t1, t2;
    auto normal_ray = Ray(ray.org, ::bvh::v2::normalize(ray.dir));

    bool is_intersect = CloestPointBetweenLines(m_line, normal_ray, intersection_point, intersection_point_on_query, t1, t2);
    if (is_intersect) {
      Scalar distance = ::bvh::v2::length(intersection_point - intersection_point_on_query);
      if(t2 < 0){ // return false when the intersection point is on the opposite direction of the query ray
        return {false, intersection_point_on_query, intersection_point, BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
      }
      if (distance < torlerance || torlerance < 0) {
        // special snap capture
        if (context.CaptureType.Contain(SnapCaptureType_Semantic::FootPoint) && context.Anchor.get_smallest_axis() != std::numeric_limits<double>::max()) {
          Vec start_to_anchor = context.Anchor - m_line.org;
          Vec start_to_end = m_line.dir;
          Scalar t = ::bvh::v2::dot(start_to_anchor, start_to_end);
          size_t  dummy = BaseType::GetDataSource();
          if (std::abs(t - t1) < torlerance) {
            return {
                true,
                intersection_point_on_query,
                m_line.org + t * start_to_end,
                BaseType::priority,
                BaseType::semantic,
                BaseType::GetDataSource(),
                SnapCaptureType_Semantic::FootPoint};
          }
        }
        if (context.CaptureType.Contain(SnapCaptureType_Semantic::MostClosedPoint)) {
          return {
              true,
              intersection_point_on_query,
              intersection_point,
              BaseType::priority,
              BaseType::semantic,
              BaseType::GetDataSource(),
              SnapCaptureType_Semantic::MostClosedPoint};
        }
      } else {
        return {
            false,
            intersection_point_on_query,
            intersection_point,
            BaseType::priority,
            BaseType::semantic,
            BaseType::GetDataSource(),
            {}};
      }
    }
    return {
        false,
        intersection_point_on_query,
        intersection_point,
        BaseType::priority,
        BaseType::semantic,
        BaseType::GetDataSource(),
        {}};
  }

  typename BaseType::POI_Int_Res IntersectByPoint(const Vec &point, const POI_Intersect_Context<Dimension> &context) {
    if (context.CaptureType.Contain(SnapCaptureType_Semantic::ForbidCapture)) {
      return {
          false, Vec(0), Vec(0), BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }

    Vec intersection_point_on_query = point;
    Vec org_to_point = point - m_line.org;
    Vec dir = ::bvh::v2::normalize(m_line.dir);
    Vec intersection_point_on_line = m_line.org + ::bvh::v2::dot(org_to_point, dir) * dir;

    Scalar distance = ::bvh::v2::length(intersection_point_on_line - intersection_point_on_query);
    if (distance < torlerance || torlerance < 0) {
      return {
          true,
          intersection_point_on_query,
          intersection_point_on_line,
          BaseType::priority,
          BaseType::semantic,
          BaseType::GetDataSource(),
          {SnapCaptureType_Semantic::MostClosedPoint}};
    } else {
      return {
          false,
          intersection_point_on_query,
          intersection_point_on_line,
          BaseType::priority,
          BaseType::semantic,
          BaseType::GetDataSource(),
          {}};
    }
  }

  static inline bool CloestPointBetweenLines(bvh::v2::Ray<Scalar, Dimension> line1,
                                      bvh::v2::Ray<Scalar, Dimension>  line2,
                                      bvh::v2::Vec<Scalar, Dimension>  &close_point_on_line1,
                                      bvh::v2::Vec<Scalar, Dimension>  &close_point_on_line2,
                                      Scalar & t1,
                                      Scalar & t2) {
    using Vec = ::bvh::v2::Vec<Scalar, 3>;
    /*
   * L1: P = P1 + t * D1
      L2: P = P2 + s * D2
      求解t和s，使得P1 + t * D1 约等于 P2 + s * D2

    写出目标函数并求偏导，得到解析方程是
        (D1 · D1)t - (D1 · D2)s = D1 · (P2 - P1)
        -(D2 · D1)t + (D2 · D2)s = D2 · (P2 - P1)
                                          * */
        Vec P1 = line1.org;
    Vec D1 = line1.dir;
    Vec P2 = line2.org;
    Vec D2 = line2.dir;

    Vec P1P2 = P2 - P1;
    Vec vecD1(D1);
    Vec vecD2(D2);

    Scalar a = ::bvh::v2::dot(vecD1, vecD1);
    Scalar b = ::bvh::v2::dot(vecD1, vecD2);
    Scalar e = ::bvh::v2::dot(vecD2, vecD2);

    Scalar d = a * e - b * b;
    if (fabs(d) < 1e-6) {
      // 直线太接近平行，无法计算
      return false;
    }

    Scalar r = ::bvh::v2::dot(vecD1, P1P2);
    Scalar s = ::bvh::v2::dot(vecD2, P1P2);

    Scalar t = (b * s - e * r) / d;
    Scalar u = (a * s - b * r) / d;

    close_point_on_line1 = P1 + (-t * vecD1);
    close_point_on_line2 = P2 + (-u * vecD2);
    t1 = -t;
    t2 = -u;
    return true;
  }
};


#endif//ROSEFINCH_POI_DATA_LINE_H

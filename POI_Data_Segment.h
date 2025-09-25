//
// Created by gmh on 24-8-16.
//

#ifndef ROSEFINCH_POI_DATA_SEGMENT_H
#define ROSEFINCH_POI_DATA_SEGMENT_H


#include "POI_Data.h"
#include "POI_Data_Line.h"


template<int Dimension, class DataSource>
class POI_Data_Segment : public POI_Data<Dimension,
                                         DataSource,
                                         POI_Data_Segment<Dimension, DataSource>> {
 public:
  using BaseType = POI_Data<Dimension, DataSource, POI_Data_Segment<Dimension, DataSource>>;
 private:
  using Scalar = double;
  using BBox = ::bvh::v2::BBox<Scalar, Dimension>;
  using Vec = ::bvh::v2::Vec<Scalar, Dimension>;
  using Ray = ::bvh::v2::Ray<Scalar, Dimension>;

 public:
  Vec m_Start;
  Vec m_End;
 private:
  Scalar torlerance = 10;

 public:
  POI_Data_Segment(const Vec &start, const Vec &end) : m_Start(start), m_End(end) {}
  POI_Data_Segment(const Vec &start, const Vec &end, double torlerance) : m_Start(start), m_End(end), torlerance(torlerance) {}

  BBox GetAABB() const {
    Scalar fix_tor = torlerance;
    if (fix_tor < 0) {
      fix_tor = 50;
    }
    Vec dir = ::bvh::v2::normalize(m_End - m_Start);
    Vec tor_start = m_Start - dir*fix_tor;
    Vec tor_end = m_End + dir*fix_tor;
    return BBox(tor_start, tor_end);
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
    Ray my_line = Ray(m_Start, ::bvh::v2::normalize(m_End - m_Start));

    bool is_intersect = POI_Data_Line<Dimension,DataSource>::CloestPointBetweenLines(my_line, normal_ray, intersection_point, intersection_point_on_query, t1, t2);
    if (is_intersect) {
      if (t2 < 0) {
        return {
            false,
            intersection_point_on_query,
            intersection_point,
            BaseType::priority,
            BaseType::semantic,
            BaseType::GetDataSource(),
            {}};
      } else if (!(::bvh::v2::length(intersection_point - intersection_point_on_query) < torlerance
                   || torlerance < 0)) {
        return {
            false,
            intersection_point_on_query,
            intersection_point,
            BaseType::priority,
            BaseType::semantic,
            BaseType::GetDataSource(),
            {}};
      }
      else if (t1 + torlerance >=0 && t1 - torlerance <= ::bvh::v2::length(m_End - m_Start)) {
        // special snap capture
        if (context.CaptureType.Contain(SnapCaptureType_Semantic::EndingPoint)) {
          if (std::abs(t1) < torlerance) {
            return {
                true,
                intersection_point_on_query,
                m_Start,
                BaseType::priority,
                BaseType::semantic,
                BaseType::GetDataSource(),
                SnapCaptureType_Semantic::EndingPoint};
          } else if (std::abs(t1 - ::bvh::v2::length(m_End - m_Start)) < torlerance) {
            return {
                true,
                intersection_point_on_query,
                m_End,
                BaseType::priority,
                BaseType::semantic,
                BaseType::GetDataSource(),
                SnapCaptureType_Semantic::EndingPoint};
          }
        }
        if (context.CaptureType.Contain(SnapCaptureType_Semantic::MidPoint)) {
          if (std::abs(t1 - ::bvh::v2::length(m_End - m_Start) / 2) < torlerance) {
            return {
                true,
                intersection_point_on_query,
                Vec((m_Start + m_End) * 0.5),
                BaseType::priority,
                BaseType::semantic,
                BaseType::GetDataSource(),
                SnapCaptureType_Semantic::MidPoint};
          }
        }
        if (context.CaptureType.Contain(SnapCaptureType_Semantic::FootPoint) && context.Anchor.get_smallest_axis() != std::numeric_limits<double>::max()) {
          Vec start_to_anchor = context.Anchor - m_Start;
          Vec start_to_end = m_End - m_Start;
          start_to_end = ::bvh::v2::normalize(start_to_end);
          Scalar t = ::bvh::v2::dot(start_to_anchor, start_to_end);
          if (t > 0 && t < ::bvh::v2::length(m_End - m_Start) && std::abs(t - t1) < torlerance) {
            return {
                true,
                intersection_point_on_query,
                m_Start + t * start_to_end,
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
      return {false, Vec(0), Vec(0), BaseType::priority, BaseType::semantic, BaseType::GetDataSource(), {}};
    }

    Vec intersection_point_on_query = point;
    Vec intersection_point;
    Ray my_line = Ray(m_Start, ::bvh::v2::normalize(m_End - m_Start));

    Vec org_to_point = point - my_line.org;
    Vec dir = ::bvh::v2::normalize(my_line.dir);
    Vec intersection_point_on_line = my_line.org + ::bvh::v2::dot(org_to_point, dir) * dir;

    Scalar distance = ::bvh::v2::length(intersection_point_on_line - intersection_point_on_query);
    if (distance < torlerance || torlerance < 0) {
      // check if the intersection point is on the segment
      Vec start_to_intersection = intersection_point_on_line - m_Start;
      Vec end_to_intersection = intersection_point_on_line - m_End;
      Scalar cos = ::bvh::v2::dot(start_to_intersection, end_to_intersection);
      if (cos <= 0) {
        return {
            true,
            intersection_point_on_query,
            intersection_point_on_line,
            BaseType::priority,
            BaseType::semantic,
            BaseType::GetDataSource(),
            SnapCaptureType_Semantic::MostClosedPoint};
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
};


#endif//ROSEFINCH_POI_DATA_SEGMENT_H

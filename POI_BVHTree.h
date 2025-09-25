//
// Created by gmh on 24-8-16.
//

#ifndef ROSEFINCH_POI_BVHTREE_H
#define ROSEFINCH_POI_BVHTREE_H

#include "bvh/v2/bvh.h"
#include "bvh/v2/default_builder.h"
#include "bvh/v2/stack.h"
#include "POI_Data_Semantic.h"
#include "POI_Data_Line.h"
#include "POI_Data_Point.h"
#include "POI_Data_Segment.h"


/*
  Graph drawing by ascii flow:

------------------------------------------
  ::bvh::v2::Bvh<Node> 的解释：

 ┌─────────────────────────────────┐
 │ ::bvh::v2::Bvh<Node>            │
 ├─────────────────────────────────┤
 │ +std::vector<Node> nodes;───────┼─► ::bvh::v2::Bvh 使用链表法来记录一颗二叉树的所有节点
 │                                 │
 │ +std::vector<size_t> prim_ids;──┼─► 用于记录生成这颗BVH树的AABB的原始数据的索引，具体关系见下文。
 └─────────────────────────────────┘

 ------------------------------------------
  ::bvh::v2::Node<Scalar, Dimension> 的解释：

 ┌─────────────────────────────────────┐
 │ ::bvh::v2::Node<Scalar, Dimension>  │
 ├─────────────────────────────────────┤
 │ bvh::v2::Index index ───────────────┼─► 基本上来说，用一个 size_of(Scalar) 的二进制数， 用分段的方式来同时表达两个内容：
 │                                     │    1. 一个节点的左子节点的索引 或 其持有的第一个prim对应的prim_id的索引 2. 持有的prim的数量
 │ std::array<T, Dim * 2> bounds ──────┼─► 用于记录这个节点的AABB包围盒，具体记录方法看原文件注释
 └─────────────────────────────────────┘

 ------------------------------------------
  bvh::v2::Index 的解释：

 Index::Value                                PrimCountBits
                                           ┌───────┴───────┐
 ┌─────────────────────────────────────────────────────────┐
 └┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┘  Scalar
 └───────────────────┬─────────────────────┘
               Index::first_id()──┬► 如果PrimCountBits为0，返回的是左子节点的索引，+1即为右子节点的索引
                                  └► 否则返回的是持有的第一个prim对应的prim_id的索引，+n (n<=PrimCountBits) 即为持有的第n个prim对应的prim_id的索引

 ------------------------------------------
  prim_ids 的使用方法:

     ┌──┬──┬──┬──┬──┬──┬──┬──┐
     │* │* │* │* │* │* │* │* │ Some_DataSource
     └──┴──┴──┴──┴──┴▲─┴──┴──┘      │
       ┌-------------┼----┐        ┌▼-------------------┐
 ┌─────►Some_DataSource[5]│        │Gen DataSource AABB │
 │     └------------------┘        └┬-------------------┘
 │   ┌──┬──┬──┬──┬──┬──┬──┬──┐      ▼
 │   │* │* │* │* │* │* │* │* │ Data_AABB
 │   └──┴──┴──┴──┴──┴▲─┴──┴──┘      │
 │              ┌----┼------─┐    ┌-▼-------------┐
 │             ┌►Data_AABB[5]│    │ Build BVHTree │
 └─────────────┤└------------┘    └-┬-------------┘
     ┌──┬──┬──┬┼─┬──┬──┬──┬──┐      ▼
     │1 │4 │2 │5 │6 │7 │3 │0 │ Bvh::nodes
     └──┴──┴──┴▲─┴──┴──┴──┴──┘
               │   ┌----------------┐
               └───│Bvh::prim_ids[3]◄────┐
        ┌─-─--─------------------┐--┘    │
        │ Node::Index::first_id()│-> 3 ──┘
        └----─-─--─--─---─-------┘

 * */

template<int Dimension, class DataSource>
class POI_BVHTree {
 private:
  using Scalar = double;
  using Node = ::bvh::v2::Node<Scalar, Dimension>;
  using BBox = ::bvh::v2::BBox<Scalar, Dimension>;
  using Vec = ::bvh::v2::Vec<Scalar, Dimension>;
  using Ray = ::bvh::v2::Ray<Scalar, Dimension>;
  using BVH = ::bvh::v2::Bvh<Node>;

  std::unique_ptr<BVH> m_BVH;
  // 已构建BVH的POI_Data缓存区
  std::vector<std::weak_ptr<POI_Data_Point<Dimension, DataSource>>> m_POI_Data_Points_cache;
  std::vector<std::weak_ptr<POI_Data_Segment<Dimension, DataSource>>> m_POI_Data_Segments_cache;

  std::vector<std::weak_ptr<POI_Data_Line<Dimension, DataSource>>> m_POI_Data_Lines_cache;

  // 未更新POI_Data暂存区
  std::vector<std::weak_ptr<POI_Data_Point<Dimension, DataSource>>> m_POI_Data_Points;
  std::vector<std::weak_ptr<POI_Data_Segment<Dimension, DataSource>>> m_POI_Data_Segments;

  std::vector<std::weak_ptr<POI_Data_Line<Dimension, DataSource>>> m_POI_Data_Lines;

  enum POI_Data_Type { Point, Segment, Line };

  std::pair<size_t, size_t> GetPrimIdRange(POI_Data_Type type) {
    size_t point_size = m_POI_Data_Points_cache.size();
    size_t seg_size = m_POI_Data_Segments_cache.size();
    size_t line_size = m_POI_Data_Lines_cache.size();
    switch (type) {
      case POI_Data_Type::Point:
        return {0, point_size};
      case POI_Data_Type::Segment:
        return {point_size, point_size + seg_size};
      case POI_Data_Type::Line:
        return {point_size + seg_size, point_size + seg_size + line_size};
      default:
        throw std::runtime_error("Invalid POI_Data_Type");
    }
  }

 public:
  void AddPOI_Data_Points(const std::vector<std::weak_ptr<POI_Data_Point<Dimension, DataSource>>> &poi_data_points) {
    m_POI_Data_Points.insert(m_POI_Data_Points.end(), poi_data_points.begin(), poi_data_points.end());
  }
  void AddPOI_Data_Lines(const std::vector<std::weak_ptr<POI_Data_Line<Dimension, DataSource>>> &poi_data_lines) {
    m_POI_Data_Lines.insert(m_POI_Data_Lines.end(), poi_data_lines.begin(), poi_data_lines.end());
  }
  void AddPOI_Data_Segments(
      const std::vector<std::weak_ptr<POI_Data_Segment<Dimension, DataSource>>> &poi_data_segments) {
    m_POI_Data_Segments.insert(m_POI_Data_Segments.end(), poi_data_segments.begin(), poi_data_segments.end());
  }

  void ClearPOI_Data_Points() {
    m_POI_Data_Points.clear();
  }
  void ClearPOI_Data_Lines() {
    m_POI_Data_Lines.clear();
  }
  void ClearPOI_Data_Segments() {
    m_POI_Data_Segments.clear();
  }

  void clearAll() {
    ClearPOI_Data_Points();
    ClearPOI_Data_Lines();
    ClearPOI_Data_Segments();
  }

  void BuildBVH(bool auto_clear = true) {
    m_POI_Data_Points_cache.clear();
    m_POI_Data_Segments_cache.clear();
    m_POI_Data_Lines_cache.clear();
    m_BVH.reset();

    // 收集POI的AABB，遇到无效的POI指针则跳过
    std::vector<BBox> aabbs;
    std::vector<Vec> centers;
    for (auto &poi_data_point : m_POI_Data_Points) {
      if (auto poi_data_point_shared = poi_data_point.lock()) {
        aabbs.push_back(poi_data_point_shared->AABB());
        centers.push_back(poi_data_point_shared->AABB().get_center());
        m_POI_Data_Points_cache.push_back(poi_data_point);
      }
    }
    for (auto &poi_data_segment : m_POI_Data_Segments) {
      if (auto poi_data_segment_shared = poi_data_segment.lock()) {
        aabbs.push_back(poi_data_segment_shared->AABB());
        centers.push_back(poi_data_segment_shared->AABB().get_center());
        m_POI_Data_Segments_cache.push_back(poi_data_segment);
      }
    }
    for (auto &poi_data_line : m_POI_Data_Lines) {
      if (auto poi_data_line_shared = poi_data_line.lock()) {
        m_POI_Data_Lines_cache.push_back(poi_data_line);
      }
    }
    if (aabbs.empty()) {
      return;
    }

    // 创建BVH
    typename ::bvh::v2::DefaultBuilder<Node>::Config config;
    config.quality = ::bvh::v2::DefaultBuilder<Node>::Quality::High;
    m_BVH = std::make_unique<BVH>(bvh::v2::DefaultBuilder<Node>::build(aabbs, centers, config));


    // 清理
    if (auto_clear)
      clearAll();
  };

  // the first Vec is the intersection point on query,
  // and the second Vec is the intersection point on the POI data.
  // int is the priority of POI data
  // POI_Data_Semantic is the POI Semantic
  using POI_Int_Res = typename POI_Data_Point<Dimension, DataSource>::BaseType::POI_Int_Res;

 private:
  template<class POI_Data_Type>
  inline POI_Int_Res Intersect(
      POI_Data_Type poi_data, const ::bvh::v2::Ray<Scalar, Dimension> &ray,
      const POI_Intersect_Context<Dimension> &context) {
    return poi_data->Intersect(ray, context);
  }

  inline POI_Int_Res Intersect(
      const ::bvh::v2::Ray<Scalar, Dimension> &query, size_t prim_id, const POI_Intersect_Context<Dimension> &context) {
    auto points_range = GetPrimIdRange(POI_Data_Type::Point);
    auto segs_range = GetPrimIdRange(POI_Data_Type::Segment);

    if (prim_id < points_range.second) {
      auto poi_data_shared = m_POI_Data_Points_cache[prim_id].lock();
      if (poi_data_shared) {
        return Intersect(poi_data_shared.get(), query, context);
      }
    } else if (prim_id >= segs_range.first && prim_id < segs_range.second) {
      auto poi_data_shared = m_POI_Data_Segments_cache[prim_id - segs_range.first].lock();
      if (poi_data_shared) {
        return Intersect(poi_data_shared.get(), query, context);
      }
    } else {
      throw std::runtime_error("Invalid prim_id");
    }
    return {false, Vec(0), Vec(0), 0, POI_Data_Semantic(), DataSource(), ""};
  }

 public:
  [[nodiscard]]
  std::vector<std::pair<size_t, POI_Int_Res>> IntersectByRay(
      const ::bvh::v2::Ray<Scalar, Dimension> &ray, const POI_Intersect_Context<Dimension> &context) {
    std::vector<std::pair<size_t, POI_Int_Res>> result;
    // query point and segment
    if (m_BVH) {
      ::bvh::v2::GrowingStack<typename BVH::Index> stack;
      auto leafFn = [this, &ray, &result, &context](const size_t first_id, const size_t last_id) -> bool {
        for (typename Node::Index::Type i = first_id; i < last_id; ++i) {
          size_t prim_id = m_BVH->prim_ids[i];
          auto hit_result = Intersect(ray, prim_id, context);
          if (std::get<0>(hit_result)) {
            result.push_back(std::make_pair(prim_id, hit_result));
          }
        }
        return false;
      };
      m_BVH->template intersect<false, true>(ray, m_BVH->get_root().index, stack, leafFn);
    }
    // query line
    auto line_range = GetPrimIdRange(POI_Data_Type::Line);
    for (int i = 0; i < m_POI_Data_Lines_cache.size(); i++) {
      if (auto poi_data_line_shared = m_POI_Data_Lines_cache[i].lock()) {
        auto hit_result = Intersect(poi_data_line_shared.get(), ray, context);
        if (std::get<0>(hit_result)) {
          auto prim_id = line_range.first + i;
          result.push_back(std::make_pair(prim_id, hit_result));
        }
      }
    }

    return result;
  };

  /*
   * POI 之间的交点计算
   * */
 public:
  std::vector<POI_Int_Res> GeometricIntersectBetweenPOI(POI_BVHTree *other, size_t prim_id, size_t other_prim_id) {
    // Get the POI_Data_Type from the prim_id
    auto points_range = GetPrimIdRange(POI_Data_Type::Point);
    auto segs_range = GetPrimIdRange(POI_Data_Type::Segment);
    auto line_range = GetPrimIdRange(POI_Data_Type::Line);
    POI_Data_Type this_POI_type;
    if (prim_id < points_range.second) {
      this_POI_type = POI_Data_Type::Point;
    } else if (prim_id >= segs_range.first && prim_id < segs_range.second) {
      this_POI_type = POI_Data_Type::Segment;
    } else if (prim_id >= line_range.first && prim_id < line_range.second) {
      this_POI_type = POI_Data_Type::Line;
    } else {
      throw std::runtime_error("Invalid prim_id");
    }

    // Get the POI_Data_Type from the other_prim_id
    auto other_points_range = other->GetPrimIdRange(POI_Data_Type::Point);
    auto other_segs_range = other->GetPrimIdRange(POI_Data_Type::Segment);
    auto other_line_range = other->GetPrimIdRange(POI_Data_Type::Line);
    POI_Data_Type other_POI_type;
    if (other_prim_id < other_points_range.second) {
      other_POI_type = POI_Data_Type::Point;
    } else if (other_prim_id >= other_segs_range.first && other_prim_id < other_segs_range.second) {
      other_POI_type = POI_Data_Type::Segment;
    } else if (other_prim_id >= other_line_range.first && other_prim_id < other_line_range.second) {
      other_POI_type = POI_Data_Type::Line;
    } else {
      throw std::runtime_error("Invalid other_prim_id");
    }

    // Call the correct implementation
    if (this_POI_type == POI_Data_Type::Segment && other_POI_type == POI_Data_Type::Segment) {
      auto this_poi = this->m_POI_Data_Segments_cache[prim_id - segs_range.first].lock();
      auto other_poi = other->m_POI_Data_Segments_cache[other_prim_id - other_segs_range.first].lock();
      return GeometricIntersectBetweenPOI_impl(this_poi.get(), other_poi.get());
    } else if (this_POI_type == POI_Data_Type::Line && other_POI_type == POI_Data_Type::Line) {
      auto this_poi = this->m_POI_Data_Lines_cache[prim_id - line_range.first].lock();
      auto other_poi = other->m_POI_Data_Lines_cache[other_prim_id - other_line_range.first].lock();
      return GeometricIntersectBetweenPOI_impl(this_poi.get(), other_poi.get());
    } else if ((this_POI_type == POI_Data_Type::Segment && other_POI_type == POI_Data_Type::Line)) {
      auto this_poi = this->m_POI_Data_Segments_cache[prim_id - segs_range.first].lock();
      auto other_poi = other->m_POI_Data_Lines_cache[other_prim_id - other_line_range.first].lock();
      return GeometricIntersectBetweenPOI_impl(this_poi.get(), other_poi.get());
    } else if ((this_POI_type == POI_Data_Type::Line && other_POI_type == POI_Data_Type::Segment)) {
      auto this_poi = this->m_POI_Data_Lines_cache[prim_id - line_range.first].lock();
      auto other_poi = other->m_POI_Data_Segments_cache[other_prim_id - other_segs_range.first].lock();
      return GeometricIntersectBetweenPOI_impl(this_poi.get(), other_poi.get());
    } else {
      return {};
    }
  }

 private:
  // line with segment
  std::vector<POI_Int_Res> GeometricIntersectBetweenPOI_impl(
      POI_Data_Segment<Dimension, DataSource> *seg, POI_Data_Line<Dimension, DataSource> *line) {
    return GeometricIntersectBetweenPOI_impl(line, seg);
  }

  std::vector<POI_Int_Res> GeometricIntersectBetweenPOI_impl(
      POI_Data_Line<Dimension, DataSource> *line, POI_Data_Segment<Dimension, DataSource> *seg) {
    std::vector<POI_Int_Res> result;

    Ray seg_as_ray(seg->m_Start, ::bvh::v2::normalize(seg->m_End - seg->m_Start));
    auto seg_length = ::bvh::v2::length(seg->m_End - seg->m_Start);
    double t1, t2;
    Vec intersection_on_seg, intersection_on_line;
    bool is_intersect = POI_Data_Line<Dimension, DataSource>::CloestPointBetweenLines(
        seg_as_ray, line->m_line, intersection_on_seg, intersection_on_line, t1, t2);
    if (is_intersect && t1 >= 0 && t1 <= seg_length
        && ::bvh::v2::length(intersection_on_seg - intersection_on_line) < 1e-3) {
      result.push_back(
          {true,
           intersection_on_line,
           intersection_on_seg,
           100,
           {},
           line->GetDataSource(),
           {SnapCaptureType_Semantic::IntersectionPoint}});
    }
    return result;
  }

  std::vector<POI_Int_Res> GeometricIntersectBetweenPOI_impl(
      POI_Data_Segment<Dimension, DataSource> *seg_1, POI_Data_Segment<Dimension, DataSource> *seg_2) {
    std::vector<POI_Int_Res> result;
    Ray this_seg_as_ray(seg_1->m_Start, ::bvh::v2::normalize(seg_1->m_End - seg_1->m_Start));
    Ray other_seg_as_ray(seg_2->m_Start, ::bvh::v2::normalize(seg_2->m_End - seg_2->m_Start));
    auto this_seg_length = ::bvh::v2::length(seg_1->m_End - seg_1->m_Start);
    auto other_seg_length = ::bvh::v2::length(seg_2->m_End - seg_2->m_Start);
    double t1, t2;
    Vec intersection_on_this_seg, intersection_on_other_seg;
    bool is_intersect = POI_Data_Line<Dimension, DataSource>::CloestPointBetweenLines(
        this_seg_as_ray, other_seg_as_ray, intersection_on_this_seg, intersection_on_other_seg, t1, t2);
    if (is_intersect && t1 >= 0 && t1 <= this_seg_length && t2 >= 0 && t2 <= other_seg_length
        && ::bvh::v2::length(intersection_on_this_seg - intersection_on_other_seg) < 1e-3) {
      result.push_back(
          {true,
           intersection_on_other_seg,
           intersection_on_this_seg,
           100,
           {},
           seg_2->GetDataSource(),
           {SnapCaptureType_Semantic::IntersectionPoint}});
    }
    return result;
  }

  // line with line
  std::vector<POI_Int_Res> GeometricIntersectBetweenPOI_impl(
      POI_Data_Line<Dimension, DataSource> *line_1, POI_Data_Line<Dimension, DataSource> *line_2) {
    std::vector<POI_Int_Res> result;
    Vec intersection_on_this_line, intersection_on_other_line;
    double t1, t2;
    bool is_intersect = POI_Data_Line<Dimension, DataSource>::CloestPointBetweenLines(
        line_1->m_line, line_2->m_line, intersection_on_this_line, intersection_on_other_line, t1, t2);
    if (is_intersect && ::bvh::v2::length(intersection_on_this_line - intersection_on_other_line) < 1e-3) {
      result.push_back(
          {true,
           intersection_on_other_line,
           intersection_on_this_line,
           100,
           {},
           line_2->GetDataSource(),
           {SnapCaptureType_Semantic::IntersectionPoint}});
    }
    return result;
  }
};


#endif//ROSEFINCH_POI_BVHTREE_H

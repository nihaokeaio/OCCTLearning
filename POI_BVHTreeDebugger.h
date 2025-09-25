//
// Created by gmh on 24-8-21.
//

#ifndef ROSEFINCH_POI_BVHTREEDEBUGGER_H
#define ROSEFINCH_POI_BVHTREEDEBUGGER_H

#include "AIS_InteractiveContext.hxx"
#include "POI_BVHTree.h"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepPrimAPI_MakeSphere.hxx"
#include "BRepPrimAPI_MakeBox.hxx"
#include "AIS_Shape.hxx"
#include "V3d_View.hxx"
#include "POI_Data_Line.h"
#include "POI_Data_Point.h"
#include "POI_Data_Segment.h"




template<int Dimension = 3, class DataSource = size_t>
class POI_BVHTreeDebugger {
 public:
  POI_BVHTreeDebugger(AIS_InteractiveContext *context, V3d_View* view) : m_Context(context), m_OcctView(view) {}

  POI_BVHTree<Dimension, DataSource> poiBvhTree;

  void AddPOI_Data_Points(const std::vector<std::shared_ptr<POI_Data_Point<Dimension, DataSource>>> &poi_data_points) {
    m_POI_Data_Points_pool.insert(m_POI_Data_Points_pool.end(), poi_data_points.begin(), poi_data_points.end());
    auto weak_p_poi_data = std::vector<std::weak_ptr<POI_Data_Point<Dimension, DataSource>>>();
    std::transform(poi_data_points.begin(), poi_data_points.end(), std::back_inserter(weak_p_poi_data),
                   [](const std::shared_ptr<POI_Data_Point<Dimension, DataSource>> &poi_data) {
                     return std::weak_ptr<POI_Data_Point<Dimension, DataSource>>(poi_data);
                   });
    poiBvhTree.AddPOI_Data_Points(weak_p_poi_data);
  }
  void AddPOI_Data_Lines(const std::vector<std::shared_ptr<POI_Data_Line<Dimension, DataSource>>> &poi_data_lines) {
    m_POI_Data_Lines_pool.insert(m_POI_Data_Lines_pool.end(), poi_data_lines.begin(), poi_data_lines.end());
    auto weak_p_poi_data = std::vector<std::weak_ptr<POI_Data_Line<Dimension, DataSource>>>();
    std::transform(poi_data_lines.begin(), poi_data_lines.end(), std::back_inserter(weak_p_poi_data),
                   [](const std::shared_ptr<POI_Data_Line<Dimension, DataSource>> &poi_data) {
                     return std::weak_ptr<POI_Data_Line<Dimension, DataSource>>(poi_data);
                   });
    poiBvhTree.AddPOI_Data_Lines(weak_p_poi_data);
  };
  void AddPOI_Data_Segments(const std::vector<std::shared_ptr<POI_Data_Segment<Dimension, DataSource>>> &poi_data_segments) {
    m_POI_Data_Segments_pool.insert(m_POI_Data_Segments_pool.end(), poi_data_segments.begin(), poi_data_segments.end());
    auto weak_p_poi_data = std::vector<std::weak_ptr<POI_Data_Segment<Dimension, DataSource>>>();
    std::transform(poi_data_segments.begin(), poi_data_segments.end(), std::back_inserter(weak_p_poi_data),
                   [](const std::shared_ptr<POI_Data_Segment<Dimension, DataSource>> &poi_data) {
                     return std::weak_ptr<POI_Data_Segment<Dimension, DataSource>>(poi_data);
                   });
    poiBvhTree.AddPOI_Data_Segments(weak_p_poi_data);
  };

  void ClearPOI_Data_Points() {
    m_POI_Data_Points_pool.clear();
    poiBvhTree.ClearPOI_Data_Points();
  }
  void ClearPOI_Data_Lines() {
    m_POI_Data_Lines_pool.clear();
    poiBvhTree.ClearPOI_Data_Lines();
  }
  void ClearPOI_Data_Segments() {
    m_POI_Data_Segments_pool.clear();
    poiBvhTree.ClearPOI_Data_Segments();
  }
  void clearAll() {
    ClearPOI_Data_Points();
    ClearPOI_Data_Lines();
    ClearPOI_Data_Segments();
  }
  void SetRay(const ::bvh::v2::Ray<double, Dimension> &ray) {
    m_Ray = ray;
  }
  void SetRay( double *org, double *dir) {
    m_Ray = ::bvh::v2::Ray<double, Dimension>(::bvh::v2::Vec<double, Dimension>(org[0], org[1], org[2]),
                                               ::bvh::v2::Vec<double, Dimension>(dir[0], dir[1], dir[2]));
  }

  void BuildBVH() {
    poiBvhTree.BuildBVH();
  }

  void ReBuildBVH() {
    poiBvhTree.BuildBVH(false);
  }

  void RebuildAndIntersect(){
    ReBuildBVH();
    auto int_res_vec = poiBvhTree.IntersectByRay(m_Ray);
    for (auto & int_res: int_res_vec){
      auto [is_intersect, intersection_point_on_query, intersection_point, priority, semantic] = int_res;
    }
  }
  typedef std::tuple<bool, ::bvh::v2::Vec<double, Dimension>, ::bvh::v2::Vec<double, Dimension>, int, POI_Data_Semantic, const DataSource &, std::string> POI_Int_Res;
  std::vector<std::pair<size_t, POI_Int_Res>> IntersectRay(const gp_Lin& ray) {
    ::bvh::v2::Ray<double, Dimension> tempRay = ::bvh::v2::Ray<double, Dimension>(::bvh::v2::Vec<double, Dimension>(ray.Location().X(),ray.Location().Y(),ray.Location().Z()),
                                               ::bvh::v2::Vec<double, Dimension>(ray.Direction().X(),ray.Direction().Y(),ray.Direction().Z()));
    POI_Intersect_Context<Dimension> context;
    context.OCC_View = m_OcctView;
    context.CaptureType.AddSemantic(SnapCaptureType_Semantic::IsolatedPoint);
    context.CaptureType.AddSemantic(SnapCaptureType_Semantic::MostClosedPoint);
    return poiBvhTree.IntersectByRay(tempRay,context);
  }

  void Render() {
    for (auto &ais : m_AIS_Shape_pool) {
      m_Context->Remove(ais, Standard_True);
    }
    for (auto &ais : m_AIS_Line_Shape_pool) {
      m_Context->Remove(ais, Standard_True);
    }
    m_AIS_Shape_pool.clear();
    m_AIS_Line_Shape_pool.clear();

    ReBuildBVH();

    {//draw ray
      gp_Pnt p1(m_Ray.org[0], m_Ray.org[1], m_Ray.org[2]);
      gp_Pnt p2(m_Ray.org[0] + m_Ray.dir[0] * 1e6, m_Ray.org[1] + m_Ray.dir[1] * 1e6, m_Ray.org[2] + m_Ray.dir[2] * 1e6);
      BRepBuilderAPI_MakeEdge edge(p1, p2);
      TopoDS_Shape ray_topo = edge.Shape();
      Handle(AIS_Shape) ray_ais = new AIS_Shape(ray_topo);
      ray_ais->SetColor(Quantity_NOC_PURPLE);
      m_Context->Display(ray_ais, Standard_True);
      m_AIS_Line_Shape_pool.push_back(ray_ais);

      // draw ray start point
      BRepPrimAPI_MakeSphere sphere(p1, 1);
      TopoDS_Shape ray_start_topo = sphere.Shape();
      Handle(AIS_Shape) ray_start_ais = new AIS_Shape(ray_start_topo);
      ray_start_ais->SetDisplayMode(AIS_Shaded);
      m_Context->Display(ray_start_ais, Standard_True);
      m_AIS_Line_Shape_pool.push_back(ray_start_ais);
    }

    //draw POI_Data_Points
    for (auto &poi_data_point : m_POI_Data_Points_pool) {
      // draw a point
      auto pt_aabb = poi_data_point->AABB();
      auto pt = pt_aabb.get_center();
      gp_Pnt p(pt[0], pt[1], pt[2]);
      BRepPrimAPI_MakeSphere sphere(p, 1);
      TopoDS_Shape pt_topo = sphere.Shape();
      Handle(AIS_Shape) pt_ais = new AIS_Shape(pt_topo);
      pt_ais->SetDisplayMode(AIS_Shaded);
      m_Context->Display(pt_ais, Standard_True);
      m_AIS_Shape_pool.push_back(pt_ais);

      gp_Pnt min(pt_aabb.min[0], pt_aabb.min[1], pt_aabb.min[2]);
      gp_Pnt max(pt_aabb.max[0], pt_aabb.max[1], pt_aabb.max[2]);
      BRepPrimAPI_MakeBox box(min, max);
      TopoDS_Shape pt_aabb_topo = box.Shape();
      Handle(AIS_Shape) pt_aabb_ais = new AIS_Shape(pt_aabb_topo);
      m_Context->Display(pt_aabb_ais, Standard_True);
      m_AIS_Shape_pool.push_back(pt_aabb_ais);
    }

    //draw POI_Data_Lines
    for (auto &poi_data_line : m_POI_Data_Lines_pool) {
      // draw a line
      auto line = poi_data_line->m_line;
      gp_Pnt p1(line.org[0] - line.dir[0]*1e6, line.org[1] - line.dir[1]*1e6, line.org[2] - line.dir[2]*1e6);
      gp_Pnt p2(line.org[0] + line.dir[0]*1e6, line.org[1] + line.dir[1]*1e6, line.org[2] + line.dir[2]*1e6);
      BRepBuilderAPI_MakeEdge edge(p1, p2);
      TopoDS_Shape line_topo = edge.Shape();
      Handle(AIS_Shape) line_ais = new AIS_Shape(line_topo);
      m_Context->Display(line_ais, Standard_True);
      m_AIS_Line_Shape_pool.push_back(line_ais);
    }

    //draw POI_Data_Segments
    for (auto &poi_data_segment : m_POI_Data_Segments_pool) {
      // draw a segment
      auto segment = poi_data_segment->m_Start;
      gp_Pnt p1(segment[0], segment[1], segment[2]);
      segment = poi_data_segment->m_End;
      gp_Pnt p2(segment[0], segment[1], segment[2]);
      BRepBuilderAPI_MakeEdge edge(p1, p2);
      TopoDS_Shape segment_topo = edge.Shape();
      Handle(AIS_Shape) segment_ais = new AIS_Shape(segment_topo);
      m_Context->Display(segment_ais, Standard_True);
      m_AIS_Shape_pool.push_back(segment_ais);

      auto segment_aabb = poi_data_segment->AABB();
      gp_Pnt min(segment_aabb.min[0], segment_aabb.min[1], segment_aabb.min[2]);
      gp_Pnt max(segment_aabb.max[0], segment_aabb.max[1], segment_aabb.max[2]);
      BRepPrimAPI_MakeBox box(min, max);
      TopoDS_Shape segment_aabb_topo = box.Shape();
      Handle(AIS_Shape) segment_aabb_ais = new AIS_Shape(segment_aabb_topo);
      m_Context->Display(segment_aabb_ais, Standard_True);
      m_AIS_Shape_pool.push_back(segment_aabb_ais);
    }

    // draw the Intersect Res
    POI_Intersect_Context<Dimension> context;
    context.OCC_View = m_OcctView;
    auto int_res_vec = poiBvhTree.IntersectByRay(m_Ray, context);
    for (auto int_res : int_res_vec) {
      auto
          [is_intersect, intersection_point_on_query, intersection_point, priority, semantic, _dataSource,
           snapCaptureType] = int_res.second;
      if (is_intersect) {
        // p on the query ray
        gp_Pnt p1(intersection_point_on_query[0], intersection_point_on_query[1], intersection_point_on_query[2]);
        BRepPrimAPI_MakeSphere sphere(p1, 2);
        TopoDS_Shape pt_topo = sphere.Shape();
        Handle(AIS_Shape) pt_ais = new AIS_Shape(pt_topo);
        pt_ais->SetColor(Quantity_NOC_GREEN);
        pt_ais->SetDisplayMode(AIS_Shaded);
        m_Context->Display(pt_ais, Standard_True);
        m_AIS_Shape_pool.push_back(pt_ais);

        // p on the POI data
        gp_Pnt p2 = gp_Pnt(intersection_point[0], intersection_point[1], intersection_point[2]);
        if (p1.Distance(p2)==0)
          continue;
        BRepPrimAPI_MakeSphere sphere2(p2, 2);
        TopoDS_Shape pt_topo2 = sphere2.Shape();
        Handle(AIS_Shape) pt_ais2 = new AIS_Shape(pt_topo2);
        pt_ais2->SetColor(Quantity_NOC_GREEN);
        pt_ais2->SetDisplayMode(AIS_Shaded);
        m_Context->Display(pt_ais2, Standard_True);
        m_AIS_Shape_pool.push_back(pt_ais2);

        // draw the line between the two points
        BRepBuilderAPI_MakeEdge edge(p1, p2);
        TopoDS_Shape line_topo = edge.Shape();
        Handle(AIS_Shape) line_ais = new AIS_Shape(line_topo);
        line_ais->SetColor(Quantity_NOC_GREEN);
        m_Context->Display(line_ais, Standard_True);
        m_AIS_Shape_pool.push_back(line_ais);
      }
    }
  }

  void FitCamera() {
    if (m_AIS_Shape_pool.empty()) {
      return;
    }
    Bnd_Box tree_aabb = m_AIS_Shape_pool.back()->BoundingBox();
    for (auto &ais : m_AIS_Shape_pool) {
      tree_aabb.Add(ais->BoundingBox());
    }
    m_OcctView->Camera()->FitMinMax(tree_aabb, 0.1, false);
  }

  void SetRayAsCarmera() {
    auto cam = m_OcctView->Camera();
    auto eye = cam->Eye();
    auto dir = cam->Direction();
    m_Ray = ::bvh::v2::Ray<double, Dimension>(::bvh::v2::Vec<double, Dimension>(eye.X(), eye.Y(), eye.Z()),
                                               ::bvh::v2::Vec<double, Dimension>(dir.X(), dir.Y(), dir.Z()));
  }

 private:
  AIS_InteractiveContext *m_Context;
  V3d_View* m_OcctView;

  ::bvh::v2::Ray<double, Dimension> m_Ray = ::bvh::v2::Ray<double, Dimension>(::bvh::v2::Vec<double, Dimension>(0, 0, 0),
                                                                               ::bvh::v2::Vec<double, Dimension>(1, 1, 1));

  std::vector<std::shared_ptr<POI_Data_Point<Dimension, DataSource>>> m_POI_Data_Points_pool;
  std::vector<std::shared_ptr<POI_Data_Segment<Dimension, DataSource>>> m_POI_Data_Segments_pool;

  std::vector<std::shared_ptr<POI_Data_Line<Dimension, DataSource>>> m_POI_Data_Lines_pool;

  std::vector<Handle(AIS_Shape)> m_AIS_Shape_pool;
  std::vector<Handle(AIS_Shape)> m_AIS_Line_Shape_pool;

  std::string intersect_info;
};


#endif//ROSEFINCH_POI_BVHTREEDEBUGGER_H

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryLayerUtils.h"
#include "MeshTriangulationUtils.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "GeometryUtils.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/fe_base.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

using namespace libMesh;

namespace BoundaryLayerUtils
{

std::unique_ptr<MeshBase>
buildBoundaryLayerRing(MeshGenerator & mg,
                       MeshBase & input_mesh,
                       const std::vector<BoundaryName> & boundary_names,
                       unsigned int num_layers,
                       Real thickness,
                       Real layer_bias,
                       bool outward,
                       const MooseEnum & tri_elem_type,
                       SubdomainID output_subdomain_id,
                       const SubdomainName & output_subdomain_name)
{
  // Extract seed boundary polyline points (vertices + optional midpoints) from input_mesh.
  std::set<std::size_t> bdry_id_set;
  if (!boundary_names.empty())
  {
    auto ids =
        MooseMeshUtils::getBoundaryIDs(input_mesh, boundary_names, /*generate unknown*/ false);
    bdry_id_set.insert(ids.begin(), ids.end());
  }
  TriangulatorInterface::MeshedHole bdry_mh(input_mesh, bdry_id_set);
  std::vector<Point> cur_pts;
  std::vector<Point> cur_mids;
  // Preserve every input boundary node (including colinear interior nodes on straight edges) so
  // the ring's innermost polyline can be stitched back to the input mesh's boundary exactly when
  // keep_input is requested by a downstream caller.
  collectExteriorVertexPointsFromMesh(bdry_mh,
                                      cur_pts,
                                      cur_mids,
                                      /*skip_node_reduction=*/true);

  // Geometric progression of incremental thicknesses. layer_thicknesses[i] is the offset
  // distance from polyline i-1 to polyline i (for i >= 1); layer_thicknesses[0] is unused.
  std::vector<Real> layer_thicknesses(num_layers + 1);
  const Real unit_thickness =
      (layer_bias == 1.0)
          ? (thickness / num_layers)
          : (thickness / (std::pow(layer_bias, num_layers) - 1.0) * (layer_bias - 1.0));
  layer_thicknesses[0] = 0.0;
  for (auto i : make_range(std::vector<Real>::size_type(1), layer_thicknesses.size()))
    layer_thicknesses[i] = unit_thickness * std::pow(layer_bias, i - 1);

  // Generate the N+1 polyline meshes. Polylines are indexed so that polyline 0 is geometrically
  // innermost (smallest) and polyline N is outermost (largest). Iteration order depends on
  // direction: for outward, build polyline 0 first (= input boundary); for inward, build polyline
  // N first.
  std::vector<std::unique_ptr<MeshBase>> polylines(num_layers + 1);
  for (auto layer_i : make_range(num_layers + 1))
  {
    const unsigned int layer_index = outward ? layer_i : (num_layers - layer_i);

    // Build the polyline mesh for storage as polylines[layer_index].
    auto ply = std::make_unique<ReplicatedMesh>(mg.comm());
    MooseMeshUtils::buildPolyLineMesh(*ply,
                                      cur_pts,
                                      cur_mids,
                                      /*loop=*/true,
                                      BoundaryName(),
                                      BoundaryName(),
                                      std::vector<unsigned int>({1}));
    polylines[layer_index] = std::move(ply);

    if (layer_i + 1 < num_layers + 1)
    {
      // Build a sacrificial polyline mesh to feed generateOffsetPolyline (it triangulates the
      // input mesh in place to compute side normals; we discard it afterwards).
      auto ply_for_offset = std::make_unique<ReplicatedMesh>(mg.comm());
      MooseMeshUtils::buildPolyLineMesh(*ply_for_offset,
                                        cur_pts,
                                        cur_mids,
                                        /*loop=*/true,
                                        BoundaryName(),
                                        BoundaryName(),
                                        std::vector<unsigned int>({1}));
      std::unique_ptr<UnstructuredMesh> ply_for_offset_u =
          dynamic_pointer_cast<UnstructuredMesh>(std::move(ply_for_offset));

      std::vector<Point> next_combined = generateOffsetPolyline(
          &mg, ply_for_offset_u, cur_pts, cur_mids, outward, layer_thicknesses[layer_i + 1]);
      if (cur_mids.empty())
        cur_pts = std::move(next_combined);
      else
      {
        const auto n_vert = cur_pts.size();
        cur_pts.assign(next_combined.begin(), next_combined.begin() + n_vert);
        cur_mids.assign(next_combined.begin() + n_vert, next_combined.end());
      }
    }
  }

  // Triangulate each annulus (between polylines[i] and polylines[i+1]). Stitch each annulus to
  // the accumulating ring (except the first one).
  std::unique_ptr<MeshBase> ring;
  for (auto i : make_range(num_layers))
  {
    MeshTriangulationUtils::XYDelaunayOptions xyd_opts;
    xyd_opts.refine_bdy = false;
    xyd_opts.verify_holes = false;
    xyd_opts.stitch_holes = {i > 0};
    xyd_opts.refine_holes = {false};
    xyd_opts.tri_elem_type = std::string(tri_elem_type);
    if (output_subdomain_id != 0)
    {
      xyd_opts.has_output_subdomain_id = true;
      xyd_opts.output_subdomain_id = output_subdomain_id;
    }
    if (output_subdomain_name.size())
    {
      xyd_opts.has_output_subdomain_name = true;
      xyd_opts.output_subdomain_name = output_subdomain_name;
    }

    std::vector<std::unique_ptr<MeshBase>> holes;
    holes.reserve(1);
    if (i == 0)
      holes.push_back(std::move(polylines[0]));
    else
      holes.push_back(std::move(ring));

    ring = MeshTriangulationUtils::triangulateWithDelaunay(
        mg, std::move(polylines[i + 1]), std::move(holes), xyd_opts);
  }
  // We now have 2 * num_layers boundaries
  // Let's only keep the innermost (1) and outermost (2 * num_layers) boundaries, and remove all
  // intermediate ring bcids
  std::vector<BoundaryID> bids_to_delete;
  for (const auto b : make_range(2 * num_layers))
  {
    if (b == 1 || b == (num_layers - 1) * 2)
      continue;
    bids_to_delete.push_back(b);
  }
  auto & bi = ring->get_boundary_info();
  for (auto b : bids_to_delete)
    bi.remove_id(b);

  return ring;
}

std::vector<Point>
generateOffsetPolyline(MeshGenerator * mg,
                       std::unique_ptr<libMesh::UnstructuredMesh> & ply_mesh_u,
                       std::vector<Point> & points,
                       std::vector<Point> & mid_points,
                       const bool outward,
                       const Real thickness)
{
  // If the input points are empty, we will extract them from the input 1D mesh
  if (points.empty())
  {
    mooseAssert(mid_points.empty(),
                "If the input points are empty, the input mid_points must be also empty.");

    TriangulatorInterface::MeshedHole bdry_mh(*ply_mesh_u);
    collectExteriorVertexPointsFromMesh(bdry_mh, points, mid_points);
  }
  // Generate a very simple triangulation mesh so that we can get the outward normal vectors
  libMesh::Poly2TriTriangulator poly2tri(*ply_mesh_u);
  poly2tri.triangulation_type() = libMesh::TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(0);
  poly2tri.set_refine_boundary_allowed(false);
  poly2tri.set_verify_hole_boundaries(false);
  poly2tri.desired_area() = 0;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false;
  if (mid_points.size())
    poly2tri.elem_type() = libMesh::ElemType::TRI6;
  poly2tri.triangulate();

  // We need to serialize the mesh for next steps
  libMesh::MeshSerializer serial(*ply_mesh_u);
  // The mesh now only contains one side set that corresponds to the outer boundary with an ID of 0
  auto bdry_list(ply_mesh_u->get_boundary_info().build_side_list());

  // For each vertex, the shifting direction to form the offset is defined by the normal vectors of
  // the two sides that contain the vertex.
  // We gather the normals for each node on the boundary here, which are all vertices because of
  // the pre-selection of nodes.
  std::map<dof_id_type, std::vector<Point>> node_normal_map;
  std::map<dof_id_type, Point> mid_node_normal_map;
  for (const auto & bside : bdry_list)
  {
    const auto & side = ply_mesh_u->elem_ptr(std::get<0>(bside))->side_ptr(std::get<1>(bside));
    // For linear elements, the side normal is constant and can be obtained straightforwardly;
    // For quadratic elements, we need the normal at the shared vertices
    const Point side_normal_0 =
        mid_points.size()
            ? getKeyNormal(ply_mesh_u->elem_ptr(std::get<0>(bside)), std::get<1>(bside), 0)
            : ply_mesh_u->elem_ptr(std::get<0>(bside))
                  ->side_vertex_average_normal(std::get<1>(bside));
    const Point side_normal_1 =
        mid_points.size()
            ? getKeyNormal(ply_mesh_u->elem_ptr(std::get<0>(bside)), std::get<1>(bside), 1)
            : side_normal_0;

    if (node_normal_map.count(side->node_ptr(0)->id()))
      node_normal_map[side->node_ptr(0)->id()].push_back(side_normal_0);
    else
      node_normal_map[side->node_ptr(0)->id()] = {side_normal_0};
    if (node_normal_map.count(side->node_ptr(1)->id()))
      node_normal_map[side->node_ptr(1)->id()].push_back(side_normal_1);
    else
      node_normal_map[side->node_ptr(1)->id()] = {side_normal_1};

    if (mid_points.size())
    {
      const Point mid_node_normal =
          getKeyNormal(ply_mesh_u->elem_ptr(std::get<0>(bside)), std::get<1>(bside), 2);
      mid_node_normal_map
          [ply_mesh_u->elem_ptr(std::get<0>(bside))->node_ptr(std::get<1>(bside) + 3)->id()] =
              mid_node_normal;
    }
  }

  std::vector<Point> mod_reduced_pts_list(points);
  for (const auto & [node_id, normal_vecs] : node_normal_map)
  {
    mooseAssert(normal_vecs.size() == 2,
                "Each vertex should be connected to exactly two sides in a polygon.");

    const Point original_pt = *(ply_mesh_u->node_ptr(node_id));
    // Form an average normal at the vertex from the two connected sides' normals
    const Point move_dir =
        (normal_vecs.front() + normal_vecs.back()).unit() * (outward ? 1.0 : -1.0);
    // Consider four points of interest to determine the moving distance
    // 1. the vertex point
    // 2. point along normal_vecs.front() from the vertex with a distance thickness
    // 3. point along normal_vecs.back() from the vertex with a distance thickness
    // 4. point along move_dir from the vertex with a distance mov_dist
    // The four points form a kite shape with its symmetry axis along 1-4 direction
    // Angle 1-2-4 and angle 1-3-4 are right angles
    // Angle 2-1-3 (i.e., theta) can be calculated using the interior product of
    // v1 = normal_vecs.front() and v2 = normal_vecs.back():
    // cos(theta) = (v1 . v2) / (|v1| * |v2|)
    // Because of the right angles, the distance between 1 and 4 can be calculated as:
    // mov_dist = thickness / cos(theta/2)
    // and cos(theta/2) = sqrt((1 + cos(theta)) / 2)
    const Real mov_dist =
        thickness / std::sqrt((1.0 + (normal_vecs.front() * normal_vecs.back()) /
                                         (normal_vecs.front().norm() * normal_vecs.back().norm())) /
                              2.0);
    mooseAssert(std::count(points.begin(), points.end(), original_pt) == 1,
                "The original point should be found exactly once in the reduced points list.");
    mod_reduced_pts_list[std::distance(points.begin(),
                                       std::find(points.begin(), points.end(), original_pt))] =
        original_pt + move_dir * mov_dist;
  }

  // To ensure no overlapping, we need to check set of four points
  // p1 and p2 should be a pair of points before and after shifting
  // p3 and p4 should be a pair of adjacent shifted points that are neither not p2
  for (const auto & i_node_1 : make_range(mod_reduced_pts_list.size()))
  {
    const Point & p1 = points[i_node_1];
    const Point & p2 = mod_reduced_pts_list[i_node_1];
    for (const auto & i_node_2 : make_range(mod_reduced_pts_list.size()))
    {
      if (i_node_2 == i_node_1 || (i_node_2 + 1) % mod_reduced_pts_list.size() == i_node_1)
        continue;
      const Point & p3 = mod_reduced_pts_list[i_node_2];
      const Point & p4 = mod_reduced_pts_list[(i_node_2 + 1) % mod_reduced_pts_list.size()];
      if (thickness > 0)
        if (geom_utils::segmentsIntersect(p1, p2, p3, p4))
          mg->paramError(
              "thickness",
              "The thickness is so large that the mesh is tangled because the offset nodes "
              "are no longer in the same order when following the original boundary. Please "
              "reduce the thickness value.");
    }
  }

  std::vector<Point> mid_mod_reduced_pts_list(mid_points.size());
  for (const auto & [node_id, normal_vec] : mid_node_normal_map)
  {
    const Point original_pt = *(ply_mesh_u->node_ptr(node_id));
    const Point move_dir = normal_vec.unit() * (outward ? 1.0 : -1.0);
    mid_mod_reduced_pts_list[std::distance(
        mid_points.begin(), std::find(mid_points.begin(), mid_points.end(), original_pt))] =
        original_pt + move_dir * thickness;
  }

  // combine mod_reduced_pts_list and mid_mod_reduced_pts_list to get the final list of points for
  // the layer mesh
  std::vector<Point> layer_pts_list(mod_reduced_pts_list);
  layer_pts_list.insert(
      layer_pts_list.end(), mid_mod_reduced_pts_list.begin(), mid_mod_reduced_pts_list.end());

  return layer_pts_list;
}

void
collectExteriorVertexPointsFromMesh(libMesh::TriangulatorInterface::MeshedHole & bdry_mh,
                                    std::vector<Point> & points,
                                    std::vector<Point> & mid_points,
                                    const bool skip_node_reduction)
{
  for (const auto i : make_range(bdry_mh.n_points()))
  {
    if (skip_node_reduction || !geom_utils::arePointsColinear(
                                   bdry_mh.point((i - 1 + bdry_mh.n_points()) % bdry_mh.n_points()),
                                   bdry_mh.point(i),
                                   bdry_mh.point((i + 1) % bdry_mh.n_points())))
    {
      points.push_back(bdry_mh.point(i));
      if (bdry_mh.n_midpoints() == 1 && skip_node_reduction)
        mid_points.push_back(bdry_mh.midpoint(0, i));
    }
  }
}

Point
getKeyNormal(const Elem * elem, const unsigned int s, const unsigned int node_index)
{
  const std::unique_ptr<const Elem> face = elem->build_side_ptr(s);
  mooseAssert(face->type() == ElemType::EDGE3,
              "Only elements with EDGE3 sides are supported in this function.");
  mooseAssert(node_index < 3,
              "The node index for an EDGE3 side should be 0, 1, or 2 (for the two vertices and the "
              "midpoint).");
  std::unique_ptr<libMesh::FEBase> fe(
      libMesh::FEBase::build(2, libMesh::FEType(elem->default_order())));
  const std::vector<Point> & normals = fe->get_normals();
  std::vector<Point> ref_pts = {face->reference_elem()->point(node_index)};
  fe->reinit(elem, s, TOLERANCE, &ref_pts);
  return normals[0];
}
}

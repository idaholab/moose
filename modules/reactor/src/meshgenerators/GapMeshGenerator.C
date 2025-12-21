//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/parsed_function.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"
#include "DelimitedFileReader.h"

#include "libmesh/mesh_serializer.h"

registerMooseObject("ReactorApp", GapMeshGenerator);

InputParameters
GapMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to create the gap based on.");

  params.addRequiredParam<Real>("thickness", "The thickness of the gap to be created.");

  params.addParam<Real>("max_elem_size", "The maximum element size for the generated gap mesh.");

  params.addClassDescription(
      "Generate a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be a "
      "connected mesh with only one outer boundary manifold. The polyline mesh generated along "
      "with the boundary of the input mesh form a gap with a specified thickness. The mesh can "
      "further be used by XYDelaunayGenerator to generate a triangulation mesh that takes the gap "
      "into account.");

  return params;
}

GapMeshGenerator::GapMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _thickness(getParam<Real>("thickness"))
{
}

std::unique_ptr<MeshBase>
GapMeshGenerator::generate()
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));

  // MeshedHole is a good tool to extract and sort boundary points
  TriangulatorInterface::MeshedHole bdry_mh(*mesh);

  // Reduce the point list to only contain vertices
  std::vector<Point> reduced_pts_list;
  for (const auto i : make_range(bdry_mh.n_points()))
  {
    if (!isPointsColinear(bdry_mh.point((i - 1 + bdry_mh.n_points()) % bdry_mh.n_points()),
                          bdry_mh.point(i),
                          bdry_mh.point((i + 1) % bdry_mh.n_points())))
      reduced_pts_list.push_back(bdry_mh.point(i));
  }
  // Here we need a method to generate the outward normals of each external side
  auto ply_mesh = buildMeshBaseObject();

  MooseMeshUtils::buildPolyLineMesh(
      *ply_mesh, reduced_pts_list, true, "dummy", "dummy", std::vector<unsigned int>({1}));

  std::unique_ptr<UnstructuredMesh> ply_mesh_u =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(ply_mesh));

  // Generate a very simple triangulation mesh so that we can get the outward normal vectors
  libMesh::Poly2TriTriangulator poly2tri(*ply_mesh_u);
  poly2tri.triangulation_type() = libMesh::TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(0);
  poly2tri.set_refine_boundary_allowed(false);
  poly2tri.set_verify_hole_boundaries(false);
  poly2tri.desired_area() = 0;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false;
  poly2tri.triangulate();

  // We need to serialize the mesh for next steps
  libMesh::MeshSerializer serial(*ply_mesh_u);
  // The mesh now only contain one side set that corresponds to the outer boundary with an ID of 0
  auto bdry_list(ply_mesh_u->get_boundary_info().build_side_list());

  // For each vertex, the shifting direction to form the gap is defined by the normal vectors of the
  // two sides that contain the vertex
  std::map<dof_id_type, std::vector<Point>> node_normal_map;
  for (const auto & bside : bdry_list)
  {
    const auto & side = ply_mesh_u->elem_ptr(std::get<0>(bside))->side_ptr(std::get<1>(bside));
    const Point side_normal =
        ply_mesh_u->elem_ptr(std::get<0>(bside))->side_vertex_average_normal(std::get<1>(bside));
    if (node_normal_map.count(side->node_ptr(0)->id()))
      node_normal_map[side->node_ptr(0)->id()].push_back(side_normal);
    else
      node_normal_map[side->node_ptr(0)->id()] = {side_normal};
    if (node_normal_map.count(side->node_ptr(1)->id()))
      node_normal_map[side->node_ptr(1)->id()].push_back(side_normal);
    else
      node_normal_map[side->node_ptr(1)->id()] = {side_normal};
  }

  std::vector<Point> mod_reduced_pts_list(reduced_pts_list);
  for (const auto & [node_id, normal_vecs] : node_normal_map)
  {
    mooseAssert(normal_vecs.size() == 2,
                "Each vertex should be connected to exactly two sides in a polygon.");

    const Point original_pt = *(ply_mesh_u->node_ptr(node_id));
    const Point move_dir = (normal_vecs.front() + normal_vecs.back()).unit();
    const Real mov_dist =
        _thickness /
        std::sqrt((1.0 + (normal_vecs.front() * normal_vecs.back()) /
                             (normal_vecs.front().norm() * normal_vecs.back().norm())) /
                  2.0);

    mooseAssert(std::count(reduced_pts_list.begin(), reduced_pts_list.end(), original_pt) == 1,
                "The original point should be found exactly once in the reduced points list.");
    mod_reduced_pts_list[std::distance(
        reduced_pts_list.begin(),
        std::find(reduced_pts_list.begin(), reduced_pts_list.end(), original_pt))] =
        original_pt + move_dir * mov_dist;
  }

  // To ensure no overlapping, we need to check set of four points
  // p1 and p2 should be the pair of points before and after shifting
  // p3 and p4 should be the pair of points after shifting a side
  for (const auto & i_node_1 : make_range(mod_reduced_pts_list.size()))
  {
    const Point & p1 = reduced_pts_list[i_node_1];
    const Point & p2 = mod_reduced_pts_list[i_node_1];
    for (const auto & i_node_2 : make_range(mod_reduced_pts_list.size()))
    {
      if (i_node_2 == i_node_1 || (i_node_2 + 1) % mod_reduced_pts_list.size() == i_node_1)
        continue;
      const Point & p3 = mod_reduced_pts_list[i_node_2];
      const Point & p4 = mod_reduced_pts_list[(i_node_2 + 1) % mod_reduced_pts_list.size()];
      if (fourPointOverlap(p1, p2, p3, p4))
        paramError("thickness",
                   "The specified thickness creates overlapping in the gap mesh. Please reduce the "
                   "thickness value.");
    }
  }

  auto ply_mesh_2 = buildMeshBaseObject();

  if (isParamValid("max_elem_size"))
    MooseMeshUtils::buildPolyLineMesh(
        *ply_mesh_2, mod_reduced_pts_list, true, "dummy", "dummy", getParam<Real>("max_elem_size"));
  else
    MooseMeshUtils::buildPolyLineMesh(
        *ply_mesh_2, mod_reduced_pts_list, true, "dummy", "dummy", std::vector<unsigned int>({1}));

  return ply_mesh_2;
}

bool
GapMeshGenerator::isPointsColinear(const Point & p1, const Point & p2, const Point & p3) const
{
  const Point v1 = p2 - p1;
  const Point v2 = p3 - p1;
  const Point cross_prod = v1.cross(v2);

  return MooseUtils::absoluteFuzzyEqual(cross_prod.norm(), 0.0);
}

bool
GapMeshGenerator::fourPointOverlap(const Point & p1,
                                   const Point & p2,
                                   const Point & p3,
                                   const Point & p4) const
{
  const Real a1 = p2(1) - p1(1);
  const Real b1 = p1(0) - p2(0);
  const Real c1 = p2(0) * p1(1) - p1(0) * p2(1);

  const Real a2 = p4(1) - p3(1);
  const Real b2 = p3(0) - p4(0);
  const Real c2 = p4(0) * p3(1) - p3(0) * p4(1);

  const Real denom = a1 * b2 - a2 * b1;
  // We should not worry about the parallel case here
  // If there is an overlap issue, it will be captured by other line segments
  if (MooseUtils::absoluteFuzzyEqual(denom, 0.0))
    return false;

  const Point intersection_pt =
      Point((b1 * c2 - b2 * c1) / denom, (a2 * c1 - a1 * c2) / denom, 0.0);

  const Real ratio_p1p2 = (intersection_pt - p1) * (p2 - p1) / ((p2 - p1).norm_sq());
  const Real ratio_p3p4 = (intersection_pt - p3) * (p4 - p3) / ((p4 - p3).norm_sq());

  if (MooseUtils::absoluteFuzzyGreaterEqual(ratio_p1p2, 0.0) &&
      MooseUtils::absoluteFuzzyLessEqual(ratio_p1p2, 1.0) &&
      MooseUtils::absoluteFuzzyGreaterEqual(ratio_p3p4, 0.0) &&
      MooseUtils::absoluteFuzzyLessEqual(ratio_p3p4, 1.0))
    return true;
  else
    return false;
}

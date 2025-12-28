//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapLineMeshGenerator.h"

#include "MooseMeshUtils.h"
#include "MooseUtils.h"
#include "GeometryUtils.h"

#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/poly2tri_triangulator.h"

registerMooseObject("ReactorApp", GapLineMeshGenerator);

InputParameters
GapLineMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to create the gap based on.");

  params.addRequiredParam<Real>("thickness", "The thickness of the gap to be created.");

  MooseEnum gap_direction("OUTWARD INWARD", "OUTWARD");

  params.addParam<MooseEnum>(
      "gap_direction",
      gap_direction,
      "In which direction the gap is created with respect to the boundary of the input mesh.");

  params.addParam<std::vector<boundary_id_type>>(
      "boundary_ids",
      std::vector<boundary_id_type>(),
      "The boundary IDs around which the gap will be created.");

  params.addParam<Real>("max_elem_size", "The maximum element size for the generated gap mesh.");

  params.addClassDescription(
      "Generate a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be a "
      "connected mesh with only one outer boundary manifold. The polyline mesh generated along "
      "with the boundary of the input mesh form an unmeshed gap with a specified thickness.");

  return params;
}

GapLineMeshGenerator::GapLineMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _thickness(getParam<Real>("thickness")),
    _gap_direction(getParam<MooseEnum>("gap_direction").template getEnum<GapDirection>()),
    _boundary_ids(getParam<std::vector<boundary_id_type>>("boundary_ids"))
{
}

std::unique_ptr<MeshBase>
GapLineMeshGenerator::generate()
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));

  std::set<std::size_t> mesh_bdry_ids(_boundary_ids.begin(), _boundary_ids.end());
  // MeshedHole is a good tool to extract and sort boundary points
  TriangulatorInterface::MeshedHole bdry_mh(*mesh, mesh_bdry_ids);

  // Reduce the point list to only contain vertices
  std::vector<Point> reduced_pts_list;
  for (const auto i : make_range(bdry_mh.n_points()))
  {
    if (!geom_utils::isPointsColinear(
            bdry_mh.point((i - 1 + bdry_mh.n_points()) % bdry_mh.n_points()),
            bdry_mh.point(i),
            bdry_mh.point((i + 1) % bdry_mh.n_points())))
      reduced_pts_list.push_back(bdry_mh.point(i));
  }
  // Here we need a method to generate the outward normals of each external side
  auto ply_mesh = buildMeshBaseObject();

  MooseMeshUtils::buildPolyLineMesh(
      *ply_mesh, reduced_pts_list, /*loop*/ true, "dummy", "dummy", std::vector<unsigned int>({1}));

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
  // The mesh now only contains one side set that corresponds to the outer boundary with an ID of 0
  auto bdry_list(ply_mesh_u->get_boundary_info().build_side_list());

  // For each vertex, the shifting direction to form the gap is defined by the normal vectors of the
  // two sides that contain the vertex
  // We gather the normals for each node on the boundary here, which are all vertices because of the
  // pre-selection of nodes
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
    // Form an average normal at the vertex from the two connected sides' normals
    const Point move_dir = (normal_vecs.front() + normal_vecs.back()).unit() *
                           ((_gap_direction == GapDirection::OUTWARD) ? 1.0 : -1.0);
    // Consider four points of interest to determine the moving distance
    // 1. the vertex point
    // 2. point along normal_vecs.front() from the vertex with a distance _thickness
    // 3. point along normal_vecs.back() from the vertex with a distance _thickness
    // 4. point along move_dir from the vertex with a distance mov_dist
    // The four points form a kite shape with its symmetry axis along 1-4 direction
    // Angle 1-2-4 and angle 1-3-4 are right angles
    // Angle 2-1-3 (i.e., theta) can be calculated using the interior product of
    // v1 = normal_vecs.front() and v2 = normal_vecs.back():
    // cos(theta) = (v1 . v2) / (|v1| * |v2|)
    // Because of the right angles, the distance between 1 and 4 can be calculated as:
    // mov_dist = _thickness / cos(theta/2)
    // and cos(theta/2) = sqrt((1 + cos(theta)) / 2)
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
  // p1 and p2 should be the pair of points before shifting the side
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
      if (geom_utils::segmentsIntersect(p1, p2, p3, p4))
        paramError("thickness",
                   "The thickness is so large that the mesh is tangled because the offset nodes "
                   "are no longer in the same order when following the original boundary. Please "
                   "reduce the thickness value.");
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

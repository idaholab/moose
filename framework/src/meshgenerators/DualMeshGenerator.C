//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualMeshGenerator.h"
#include "Conversion.h"
#include "CastUniquePointer.h"
#include "GeometryUtils.h"
#include "LineSegment.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/node_elem.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/type_vector.h"
#include "libmesh/elem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

registerMooseObject("MooseApp", DualMeshGenerator);

InputParameters
DualMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  MooseEnum dual_mesh_type("voronoi barycentric", "barycentric");
  params.addParam<MooseEnum>("dual_mesh_type",
                             dual_mesh_type,
                             "Whether to output a barycentric dual or a Voronoi dual of the "
                             "Delaunay triangulation of the primal mesh.");
  params.addRangeCheckedParam<Real>(
      "boundary_node_angular_tol",
      1e-8,
      "boundary_node_angular_tol>=0",
      "Angular tolerance, in radians, used to decide whether a primal boundary node is collinear "
      "with its two adjacent boundary edges. Nodes whose boundary angle differs from pi by more "
      "than this tolerance are treated as primal boundary vertices.");
  params.addRangeCheckedParam<Real>(
      "geometry_relative_tol",
      1e-12,
      "geometry_relative_tol>=0",
      "Relative tolerance used for geometric point comparison, intersection, and area checks. The "
      "generator scales this value by the input mesh bounding-box size.");
  params.addParam<bool>(
      "preserve_subdomain_interfaces",
      false,
      "Whether the dual construction should treat interfaces between primal subdomains like "
      "preserved boundary surfaces.");
  params.addParam<std::vector<SubdomainName>>(
      "preserve_primal_subdomains",
      {},
      "Subdomains to keep as primal elements while dualizing the rest of the mesh.");
  params.addClassDescription("Takes a 2D mesh as input and returns a dual mesh, i.e., "
                             "changes each input node into an element and each input element "
                             "into a node located at its circumcenter or centroid.");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _dual_mesh_type(getParam<MooseEnum>("dual_mesh_type")),
    _boundary_node_angular_tol(getParam<Real>("boundary_node_angular_tol")),
    _geometry_relative_tol(getParam<Real>("geometry_relative_tol")),
    _preserve_subdomain_interfaces(getParam<bool>("preserve_subdomain_interfaces")),
    _preserve_primal_subdomains(getParam<std::vector<SubdomainName>>("preserve_primal_subdomains"))
{
}

std::set<SubdomainID>
DualMeshGenerator::preservedPrimalSubdomainIDs(const MeshBase & input_mesh) const
{
  std::set<SubdomainID> preserve_primal_subdomain_ids;

  if (_preserve_primal_subdomains.empty())
    return preserve_primal_subdomain_ids;

  for (const auto & name : _preserve_primal_subdomains)
    if (!MooseMeshUtils::hasSubdomainName(input_mesh, name))
      paramError("preserve_primal_subdomains", "The block '", name, "' was not found in the mesh");

  const auto ids = MooseMeshUtils::getSubdomainIDs(input_mesh, _preserve_primal_subdomains);
  preserve_primal_subdomain_ids.insert(ids.begin(), ids.end());
  return preserve_primal_subdomain_ids;
}

// True only when two elements share a full edge.
static bool
elementsShareTwoNodes(const Elem * a, const Elem * b)
{
  unsigned int shared_nodes = 0;

  for (const auto i : a->node_index_range())
    for (const auto j : b->node_index_range())
      if (a->node_id(i) == b->node_id(j))
        ++shared_nodes;

  mooseAssert(
      (shared_nodes < 3),
      "Detected elements sharing > 3 nodes."); // This is typically a sign of intertwined elements

  return shared_nodes == 2;
}

// Cross product, but includes sign for orientation
static Real
cross2D(const Point & a, const Point & b, const Point & c)
{
  return (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0));
}

// Adds points but prevents adding duplicates
static void
addUniquePoint(std::vector<Point> & points, const Point & point, const Real length_tol = 1e-12)
{
  for (const auto & existing_point : points)
    if (MooseUtils::absoluteFuzzyEqual(existing_point, point, length_tol))
      return;

  points.push_back(point);
}

// Detemines if `point` lies on the line segment from a to b
static bool
pointOnSegment2D(const Point & point, const Point & a, const Point & b)
{
  if (a.absolute_fuzzy_equals(b))
    return a.absolute_fuzzy_equals(point);

  return LineSegment(a, b).contains_point(point);
}

static bool
pointOnSegment2D(const Point & point, const Point & a, const Point & b, const Real, const Real)
{
  return pointOnSegment2D(point, a, b);
}

// We use intersections when we clip a mesh back to the primal boundary. Those intersection points
// are necessary to keep the elements being reasonable within the boundary
static void
addSegmentIntersections2D(std::vector<Point> & points,
                          const Point & p0,
                          const Point & p1,
                          const Point & q0,
                          const Point & q1,
                          const Real length_tol,
                          const Real,
                          const Real)
{
  if (p0.absolute_fuzzy_equals(p1))
  {
    if (pointOnSegment2D(p0, q0, q1))
      addUniquePoint(points, p0, length_tol);

    return;
  }

  if (q0.absolute_fuzzy_equals(q1))
  {
    if (pointOnSegment2D(q0, p0, p1))
      addUniquePoint(points, q0, length_tol);

    return;
  }

  const LineSegment segment0(p0, p1);
  const LineSegment segment1(q0, q1);

  if (geom_utils::arePointsColinear(p0, p1, q0) && geom_utils::arePointsColinear(p0, p1, q1))
  {
    if (pointOnSegment2D(q0, p0, p1))
      addUniquePoint(points, q0, length_tol);
    if (pointOnSegment2D(q1, p0, p1))
      addUniquePoint(points, q1, length_tol);
    if (pointOnSegment2D(p0, q0, q1))
      addUniquePoint(points, p0, length_tol);
    if (pointOnSegment2D(p1, q0, q1))
      addUniquePoint(points, p1, length_tol);

    return;
  }

  Point intersection = p0;

  if (segment0.intersect(segment1, intersection) && pointOnSegment2D(intersection, p0, p1) &&
      pointOnSegment2D(intersection, q0, q1))
    addUniquePoint(points, intersection, length_tol);
}

// Ray casting test to see if points are inside a polygon -- libmesh util is more limited
static bool
pointInPolygon2D(const Point & point, const std::vector<Point> & polygon)
{
  if (polygon.size() < 3)
    return false;

  bool inside = false;

  for (const auto i : index_range(polygon))
  {
    const auto j = (i + polygon.size() - 1) % polygon.size();
    const Point & pi = polygon[i];
    const Point & pj = polygon[j];

    if (pointOnSegment2D(point, pj, pi))
      return true;

    if ((pi(1) > point(1)) != (pj(1) > point(1)) &&
        point(0) < (pj(0) - pi(0)) * (point(1) - pi(1)) / (pj(1) - pi(1)) + pi(0))
      inside = !inside;
  }

  return inside;
}

static bool
pointInPolygon2D(const Point & point, const std::vector<Point> & polygon, const Real, const Real)
{
  return pointInPolygon2D(point, polygon);
}

static Real
polygonSignedArea2D(const std::vector<Point> & polygon)
{
  if (polygon.size() < 3)
    return 0.0;

  Real area = 0.0;

  for (const auto i : index_range(polygon))
  {
    const auto j = (i + polygon.size() - 1) % polygon.size();
    area += polygon[j](0) * polygon[i](1) - polygon[i](0) * polygon[j](1);
  }

  return 0.5 * area;
}

struct BoundarySegment
{
  dof_id_type node0;
  dof_id_type node1;
  Point p0;
  Point p1;
};

// Helps maps treat an edge the same regardless of direction
static std::pair<dof_id_type, dof_id_type>
edgeKey(const dof_id_type node0, const dof_id_type node1)
{
  return {std::min(node0, node1), std::max(node0, node1)};
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  auto input_mesh = std::move(_input);

  if (!input_mesh->is_prepared())
    input_mesh->find_neighbors();

  if (input_mesh->mesh_dimension() != 2)
    mooseError("DualMeshGenerator currently only supports 2D Meshes");

  const bool use_voronoi = _dual_mesh_type == "voronoi";

  if (use_voronoi && _preserve_subdomain_interfaces)
  {
    std::set<SubdomainID> subdomain_ids;

    for (const auto & elem : input_mesh->element_ptr_range())
      subdomain_ids.insert(elem->subdomain_id());

    if (subdomain_ids.size() > 1)
      paramError("preserve_subdomain_interfaces",
                 "DualMeshGenerator does not support preserving subdomain interfaces with "
                 "Voronoi duals for meshes with multiple subdomains.");
  }

  if (use_voronoi && !_preserve_primal_subdomains.empty())
    paramError("preserve_primal_subdomains",
               "Preserving primal subdomains is currently only implemented for barycentric 2D "
               "duals.");

  const auto preserve_primal_subdomain_ids = preservedPrimalSubdomainIDs(*input_mesh);

  // More subdomain helpers
  using NodeSubdomainKey = std::pair<dof_id_type, SubdomainID>;
  using RoundedPointKey2D = std::array<long long, 2>;
  struct BoundaryRegion2D
  {
    std::map<dof_id_type, Point> boundary_node_points;
    std::map<dof_id_type, std::vector<Point>> boundary_node_midpoints;
    std::vector<BoundarySegment> preserved_boundary_segments;
    std::unordered_map<dof_id_type, std::vector<std::size_t>> boundary_node_to_segments;
    std::unordered_set<dof_id_type> boundary_vertex_nodes;
    std::vector<std::vector<Point>> boundary_point_loops;
    std::vector<std::pair<Point, Point>> boundary_clip_segments;
  };
  const SubdomainID merged_subdomain_key = Elem::invalid_subdomain_id;
  const auto preservePrimalSubdomain = [&](const SubdomainID subdomain_id)
  { return preserve_primal_subdomain_ids.count(subdomain_id) > 0; };
  const auto dualizeElem = [&](const Elem & elem)
  { return !preservePrimalSubdomain(elem.subdomain_id()); };
  const auto subdomainKey = [&](const SubdomainID subdomain_id)
  { return _preserve_subdomain_interfaces ? subdomain_id : merged_subdomain_key; };
  const auto nodeSubdomainKey = [&](const dof_id_type node_id, const SubdomainID subdomain_id)
  { return NodeSubdomainKey{node_id, subdomainKey(subdomain_id)}; };
  const auto preservedSide = [&](const Elem & elem, const unsigned int side)
  {
    const auto * const neighbor = elem.neighbor_ptr(side);
    return neighbor == nullptr ||
           (neighbor != nullptr && preservePrimalSubdomain(neighbor->subdomain_id())) ||
           (_preserve_subdomain_interfaces && neighbor->subdomain_id() != elem.subdomain_id());
  };

  // Scaling tolerances
  const auto input_bounding_box = MeshTools::create_bounding_box(*input_mesh);
  const Point mesh_extent = input_bounding_box.max() - input_bounding_box.min();
  const Real mesh_scale = std::max(std::max(std::abs(mesh_extent(0)), std::abs(mesh_extent(1))),
                                   std::numeric_limits<Real>::min());
  const Real length_tol = _geometry_relative_tol * mesh_scale;
  const Real area_tol = _geometry_relative_tol * mesh_scale * mesh_scale;
  const Real parameter_tol = _geometry_relative_tol;

  std::map<SubdomainID, BoundaryRegion2D> boundary_regions;

  // Looping through primal elements
  for (const auto & elem : input_mesh->element_ptr_range())
  {
    if (!dualizeElem(*elem))
      continue;

    auto & boundary_region = boundary_regions[subdomainKey(elem->subdomain_id())];

    for (const auto side : elem->side_index_range())
    {
      if (preservedSide(*elem, side))
      {
        // If element has nullptr neighbor we want to record vertices, midpoints, and other info
        auto side_elem = elem->build_side_ptr(side);

        if (side_elem->n_nodes() == 2)
        {
          const dof_id_type node0 = side_elem->node_id(0);
          const dof_id_type node1 = side_elem->node_id(1);

          boundary_region.preserved_boundary_segments.push_back(
              {node0, node1, side_elem->point(0), side_elem->point(1)});

          boundary_region.boundary_node_points[node0] = side_elem->point(0);
          boundary_region.boundary_node_points[node1] = side_elem->point(1);

          const Point midpoint = 0.5 * (side_elem->point(0) + side_elem->point(1));
          boundary_region.boundary_node_midpoints[node0].push_back(midpoint);
          boundary_region.boundary_node_midpoints[node1].push_back(midpoint);
        }
      }
    }
  }

  // Building map for boundary segments
  // Helper so given a node we can get another node than shares a line segment with it
  const auto otherBoundaryNode = [&](const BoundarySegment & segment,
                                     const dof_id_type node_id) -> dof_id_type
  { return segment.node0 == node_id ? segment.node1 : segment.node0; };
  for (auto & boundary_region_it : boundary_regions)
  {
    auto & boundary_region = boundary_region_it.second;

    for (const auto i : index_range(boundary_region.preserved_boundary_segments))
    {
      boundary_region
          .boundary_node_to_segments[boundary_region.preserved_boundary_segments[i].node0]
          .push_back(i);
      boundary_region
          .boundary_node_to_segments[boundary_region.preserved_boundary_segments[i].node1]
          .push_back(i);
    }

    for (const auto & node_segments : boundary_region.boundary_node_to_segments)
    {
      const dof_id_type node_id = node_segments.first;
      const auto & segment_ids = node_segments.second;

      if (segment_ids.size() != 2)
      {
        boundary_region.boundary_vertex_nodes.insert(node_id);
        continue;
      }

      const Point & p = boundary_region.boundary_node_points[node_id];
      const Point v0 = boundary_region.boundary_node_points[otherBoundaryNode(
                           boundary_region.preserved_boundary_segments[segment_ids[0]], node_id)] -
                       p;
      const Point v1 = boundary_region.boundary_node_points[otherBoundaryNode(
                           boundary_region.preserved_boundary_segments[segment_ids[1]], node_id)] -
                       p;

      const Real norm_product = v0.norm() * v1.norm();

      if (norm_product < length_tol * length_tol)
      {
        boundary_region.boundary_vertex_nodes.insert(node_id);
        continue;
      }

      Real cos_angle = (v0 * v1) / norm_product;
      cos_angle = std::max(Real(-1.0), std::min(Real(1.0), cos_angle));

      if (std::abs(libMesh::pi - std::acos(cos_angle)) > _boundary_node_angular_tol)
        boundary_region.boundary_vertex_nodes.insert(
            node_id); // Records vertices based on tolerance criteria
    }

    std::vector<bool> used_boundary_segments(boundary_region.preserved_boundary_segments.size(),
                                             false);
    // We have a collection of boundary nodes, etc... We need to get them into a uniform order:
    const auto traceBoundaryLoop = [&](const std::size_t start_segment_id)
    {
      std::vector<dof_id_type> ordered_boundary_nodes;
      std::size_t current_segment_id = start_segment_id;
      dof_id_type current_node =
          boundary_region.preserved_boundary_segments[start_segment_id].node0;

      while (current_segment_id < boundary_region.preserved_boundary_segments.size() &&
             !used_boundary_segments[current_segment_id])
      {
        used_boundary_segments[current_segment_id] = true;

        const auto & segment = boundary_region.preserved_boundary_segments[current_segment_id];
        const dof_id_type next_node = otherBoundaryNode(segment, current_node);

        ordered_boundary_nodes.push_back(current_node);

        const auto node_segment_it = boundary_region.boundary_node_to_segments.find(next_node);

        if (node_segment_it == boundary_region.boundary_node_to_segments.end())
        {
          ordered_boundary_nodes.push_back(next_node);
          break;
        }

        std::size_t next_segment_id = boundary_region.preserved_boundary_segments.size();

        for (const auto candidate_segment_id : node_segment_it->second)
          if (!used_boundary_segments[candidate_segment_id])
          {
            next_segment_id = candidate_segment_id;
            break;
          }

        current_node = next_node;
        current_segment_id = next_segment_id;
      }

      if (!ordered_boundary_nodes.empty() && ordered_boundary_nodes.front() != current_node)
        ordered_boundary_nodes.push_back(current_node);

      return ordered_boundary_nodes;
    };

    // Over every boundary segment, we start a boundary trace, walking connected segments, and
    // converting point IDs to points. Then we record these points.
    for (const auto segment_id : index_range(boundary_region.preserved_boundary_segments))
      if (!used_boundary_segments[segment_id])
      {
        const auto ordered_boundary_nodes = traceBoundaryLoop(segment_id);

        if (ordered_boundary_nodes.size() < 2)
          continue;

        std::vector<Point> ordered_boundary_points;
        ordered_boundary_points.reserve(ordered_boundary_nodes.size());

        for (const auto node_id : ordered_boundary_nodes)
          ordered_boundary_points.push_back(boundary_region.boundary_node_points[node_id]);

        if (ordered_boundary_points.size() > 1 &&
            MooseUtils::absoluteFuzzyEqual(
                ordered_boundary_points.front(), ordered_boundary_points.back(), length_tol))
          ordered_boundary_points.pop_back();

        if (!ordered_boundary_points.empty())
          boundary_region.boundary_point_loops.push_back(ordered_boundary_points);
      }

    // Breaks down boundaries into clip segments, so when we go to clip them, we just cut between
    // primal vertices
    for (const auto & boundary_point_loop : boundary_region.boundary_point_loops)
    {
      if (boundary_point_loop.size() < 2)
        continue;

      std::size_t start_i = boundary_point_loop.size();

      for (const auto i : index_range(boundary_point_loop))
      {
        const auto node_it =
            std::find_if(boundary_region.boundary_node_points.begin(),
                         boundary_region.boundary_node_points.end(),
                         [&](const auto & boundary_node_point)
                         {
                           return MooseUtils::absoluteFuzzyEqual(
                               boundary_node_point.second, boundary_point_loop[i], length_tol);
                         });

        if (node_it != boundary_region.boundary_node_points.end() &&
            boundary_region.boundary_vertex_nodes.count(node_it->first))
        {
          start_i = i;
          break;
        }
      }

      if (start_i == boundary_point_loop.size())
      {
        for (const auto i : index_range(boundary_point_loop))
          boundary_region.boundary_clip_segments.push_back(
              {boundary_point_loop[i], boundary_point_loop[(i + 1) % boundary_point_loop.size()]});

        continue;
      }

      std::size_t clip_start_i = start_i;
      std::size_t current_i = start_i;

      while (true)
      {
        const std::size_t next_i = (current_i + 1) % boundary_point_loop.size();
        bool reached_break = next_i == clip_start_i;

        if (!reached_break)
        {
          const auto node_it =
              std::find_if(boundary_region.boundary_node_points.begin(),
                           boundary_region.boundary_node_points.end(),
                           [&](const auto & boundary_node_point)
                           {
                             return MooseUtils::absoluteFuzzyEqual(boundary_node_point.second,
                                                                   boundary_point_loop[next_i],
                                                                   length_tol);
                           });

          reached_break = node_it != boundary_region.boundary_node_points.end() &&
                          boundary_region.boundary_vertex_nodes.count(node_it->first);
        }

        if (reached_break)
        {
          boundary_region.boundary_clip_segments.push_back(
              {boundary_point_loop[clip_start_i], boundary_point_loop[next_i]});

          if (next_i == start_i)
            break;

          clip_start_i = next_i;
        }

        current_i = next_i;
      }
    }
  }

  const auto pointInsideBoundaryRegion =
      [&](const BoundaryRegion2D & boundary_region, const Point & point)
  {
    if (boundary_region.boundary_point_loops.empty())
      return true;

    bool inside = false;

    for (const auto & boundary_point_loop : boundary_region.boundary_point_loops)
      if (pointInPolygon2D(point, boundary_point_loop, length_tol, area_tol))
        inside = !inside;

    return inside;
  };

  // Now for actually finding the dual
  std::vector<Point> dual_centers;
  std::unordered_map<dof_id_type, dof_id_type> source_elem_to_center_id;
  std::map<NodeSubdomainKey, std::vector<const Elem *>> source_node_to_elems;
  std::map<dof_id_type, std::set<SubdomainID>> source_node_to_subdomains;
  std::unique_ptr<ReplicatedMesh> tri_mesh;

  for (const auto & elem : input_mesh->element_ptr_range())
  {
    if (!dualizeElem(*elem))
      continue;

    // Walk through the nodes and record them
    for (const auto n : make_range(elem->n_nodes()))
      source_node_to_subdomains[elem->node_id(n)].insert(elem->subdomain_id());
  }

  if (use_voronoi)
  {
    std::map<dof_id_type, std::vector<const Elem *>> source_node_to_tri_elems;

    // tri_mesh input is the pure primal mesh, which we pass to the Delaunay poly2Tri triangualtor
    tri_mesh = buildReplicatedMesh(2);

    for (const auto & node : input_mesh->node_ptr_range())
    {
      // Copy over all the nodes and elems from the primal
      Node * new_node = tri_mesh->add_point(*node);

      auto node_elem = std::make_unique<NodeElem>();
      node_elem->set_node(0) = new_node;
      tri_mesh->add_elem(std::move(node_elem));
    }

    // We're circumscribing this inside a large box that is scaled to the bounding box of the primal
    // mesh, then we'll take the Delaunay triangulation
    const Real outer_padding = 10.0 * mesh_scale;
    const Point outer_min(input_bounding_box.min()(0) - outer_padding,
                          input_bounding_box.min()(1) - outer_padding,
                          0.0);
    const Point outer_max(input_bounding_box.max()(0) + outer_padding,
                          input_bounding_box.max()(1) + outer_padding,
                          0.0);

    Node * p0 = tri_mesh->add_point(Point(outer_min(0), outer_min(1), 0.0));
    Node * p1 = tri_mesh->add_point(Point(outer_max(0), outer_min(1), 0.0));
    Node * p2 = tri_mesh->add_point(Point(outer_max(0), outer_max(1), 0.0));
    Node * p3 = tri_mesh->add_point(Point(outer_min(0), outer_max(1), 0.0));

    auto big_square = std::make_unique<Quad4>();

    big_square->set_node(0) = p0;
    big_square->set_node(1) = p1;
    big_square->set_node(2) = p2;
    big_square->set_node(3) = p3;

    tri_mesh->add_elem(std::move(big_square));

    Poly2TriTriangulator triangulator(dynamic_cast<UnstructuredMesh &>(*tri_mesh));
    triangulator.triangulation_type() = libMesh::TriangulatorInterface::PSLG;
    triangulator.minimum_angle() = 0;
    triangulator.desired_area() = 0;

    // The triangulator has no shortage of input parameters. This configuration does best.
    triangulator.insert_extra_points() = false;     // Don't add new nodes!
    triangulator.smooth_after_generating() = false; // Don't mess up geometry

    triangulator.triangulate();

    // Now for Voronoi, we loop over the triangles and get the circumcenters, etc
    for (const auto & tri_elem : tri_mesh->element_ptr_range())
    {
      if (tri_elem->n_vertices() != 3)
        continue;

      const dof_id_type center_id = dual_centers.size();

      dual_centers.push_back(
          libMesh::circumcenter(tri_elem->point(0), tri_elem->point(1), tri_elem->point(2)));
      source_elem_to_center_id[tri_elem->id()] = center_id;

      for (const auto n : make_range(tri_elem->n_nodes()))
        source_node_to_tri_elems[tri_elem->node_id(n)].push_back(tri_elem);
    }

    if (_preserve_subdomain_interfaces)
    {
      for (const auto & node_subdomains : source_node_to_subdomains)
      {
        const auto tri_elem_it = source_node_to_tri_elems.find(node_subdomains.first);

        if (tri_elem_it == source_node_to_tri_elems.end())
          continue;

        for (const auto subdomain_id : node_subdomains.second)
          source_node_to_elems[nodeSubdomainKey(node_subdomains.first, subdomain_id)] =
              tri_elem_it->second;
      }
    }
    else
      for (const auto & node_tri_elems : source_node_to_tri_elems)
        source_node_to_elems[nodeSubdomainKey(node_tri_elems.first, merged_subdomain_key)] =
            node_tri_elems.second;
  }
  else // For barycentric, loop over elements and get centroids
  {
    for (const auto & elem : input_mesh->element_ptr_range())
    {
      if (!dualizeElem(*elem))
        continue;

      const dof_id_type center_id = dual_centers.size();

      dual_centers.push_back(elem->true_centroid());
      source_elem_to_center_id[elem->id()] = center_id;

      for (const auto n : make_range(elem->n_nodes()))
        source_node_to_elems[nodeSubdomainKey(elem->node_id(n), elem->subdomain_id())].push_back(
            elem);
    }
  }

  // Now we've gathered our centers and boundary info, we can now build the dual
  auto dualMesh = buildReplicatedMesh(2);
  std::map<RoundedPointKey2D, std::vector<Node *>> dual_nodes_by_key;
  const Real dual_node_tol = std::max(length_tol, Real(1e-12));
  const auto roundedPointKey = [&](const Point & point)
  {
    return RoundedPointKey2D{{static_cast<long long>(std::llround(point(0) / dual_node_tol)),
                              static_cast<long long>(std::llround(point(1) / dual_node_tol))}};
  };
  const auto getDualNode = [&](const Point & point)
  {
    auto & candidate_nodes = dual_nodes_by_key[roundedPointKey(point)];

    for (auto * const node : candidate_nodes)
      if (MooseUtils::absoluteFuzzyEqual(*node, point, dual_node_tol))
        return node;

    Node * const node = dualMesh->add_point(point);
    candidate_nodes.push_back(node);
    return node;
  };
  const auto copyPreservedPrimalElements = [&]()
  {
    if (preserve_primal_subdomain_ids.empty())
      return;

    const BoundaryInfo & input_boundary_info = input_mesh->get_boundary_info();
    BoundaryInfo & boundary_info = dualMesh->get_boundary_info();
    std::map<dof_id_type, Node *> copied_nodes;

    const auto & input_sideset_map = input_boundary_info.get_sideset_name_map();
    const auto & input_nodeset_map = input_boundary_info.get_nodeset_name_map();

    // Nodeset preservation
    if (!input_sideset_map.empty())
      boundary_info.set_sideset_name_map().insert(input_sideset_map.begin(),
                                                  input_sideset_map.end());
    if (!input_nodeset_map.empty())
      boundary_info.set_nodeset_name_map().insert(input_nodeset_map.begin(),
                                                  input_nodeset_map.end());

    const auto copyNodeBoundaryIDs = [&](Node * const target_node, const dof_id_type source_node_id)
    {
      std::vector<boundary_id_type> ids_to_copy;
      input_boundary_info.boundary_ids(input_mesh->node_ptr(source_node_id), ids_to_copy);

      if (!ids_to_copy.empty())
        boundary_info.add_node(target_node, ids_to_copy);
    };

    const auto copySideBoundaryIDs = [&](const Elem * const source_elem,
                                         const unsigned int source_side,
                                         Elem * const target_elem,
                                         const std::vector<unsigned short> & target_sides)
    {
      std::vector<boundary_id_type> ids_to_copy;
      input_boundary_info.boundary_ids(source_elem, source_side, ids_to_copy);

      if (ids_to_copy.empty())
        return;

      for (const auto target_side : target_sides)
        boundary_info.add_side(target_elem, target_side, ids_to_copy);
    };

    // Copies preserved subdomain elements into the output mesh
    for (const auto & elem : input_mesh->element_ptr_range())
    {
      if (dualizeElem(*elem))
        continue;

      std::map<std::pair<dof_id_type, dof_id_type>, Point> interface_edge_midpoints;

      for (const auto side : elem->side_index_range())
      {
        const auto * const neighbor = elem->neighbor_ptr(side);

        if (neighbor == nullptr || !dualizeElem(*neighbor))
          continue;

        auto side_elem = elem->build_side_ptr(side);

        if (side_elem->n_nodes() == 2)
          interface_edge_midpoints[edgeKey(side_elem->node_id(0), side_elem->node_id(1))] =
              0.5 * (side_elem->point(0) + side_elem->point(1));
      }

      const auto getPreservedPrimalNode = [&](const dof_id_type source_node_id, const Point & point)
      {
        const auto copied_node_it = copied_nodes.find(source_node_id);

        if (copied_node_it != copied_nodes.end())
          return copied_node_it->second;

        Node * const node = getDualNode(point);
        copied_nodes[source_node_id] = node;
        copyNodeBoundaryIDs(node, source_node_id);
        return node;
      };

      if (!interface_edge_midpoints.empty())
      {
        std::vector<Node *> polygon_nodes;
        std::map<std::pair<dof_id_type, dof_id_type>, std::vector<unsigned short>>
            edge_to_polygon_sides;

        for (const auto n : make_range(elem->n_vertices()))
        {
          const dof_id_type source_node_id = elem->node_id(n);
          const auto next_n = (n + 1) % elem->n_vertices();
          const auto edge = edgeKey(source_node_id, elem->node_id(next_n));

          polygon_nodes.push_back(getPreservedPrimalNode(source_node_id, elem->point(n)));
          edge_to_polygon_sides[edge].push_back(cast_int<unsigned short>(polygon_nodes.size() - 1));

          const auto midpoint_it = interface_edge_midpoints.find(edge);

          if (midpoint_it != interface_edge_midpoints.end())
          {
            polygon_nodes.push_back(getDualNode(midpoint_it->second));
            edge_to_polygon_sides[edge].push_back(
                cast_int<unsigned short>(polygon_nodes.size() - 1));
          }
        }

        auto primal_elem = std::make_unique<libMesh::C0Polygon>(polygon_nodes.size());

        for (const auto n : index_range(polygon_nodes))
          primal_elem->set_node(n) = polygon_nodes[n];

        primal_elem->subdomain_id() = elem->subdomain_id();
        Elem * const added_elem = dualMesh->add_elem(std::move(primal_elem));

        for (const auto side : elem->side_index_range())
        {
          auto side_elem = elem->build_side_ptr(side);

          if (side_elem->n_nodes() != 2)
            continue;

          const auto side_it =
              edge_to_polygon_sides.find(edgeKey(side_elem->node_id(0), side_elem->node_id(1)));

          if (side_it != edge_to_polygon_sides.end())
            copySideBoundaryIDs(elem, side, added_elem, side_it->second);
        }

        continue;
      }

      auto primal_elem = elem->build(elem->type());

      for (const auto n : elem->node_index_range())
        primal_elem->set_node(n) = getPreservedPrimalNode(elem->node_id(n), elem->point(n));

      primal_elem->subdomain_id() = elem->subdomain_id();
      Elem * const added_elem = dualMesh->add_elem(std::move(primal_elem));

      for (const auto side : elem->side_index_range())
        copySideBoundaryIDs(elem, side, added_elem, {cast_int<unsigned short>(side)});
    }
  };

  copyPreservedPrimalElements();
  // Helper to determine if points are within the primal boundary
  const auto pointInsideBoundary =
      [&](const BoundaryRegion2D & boundary_region, const Point & point)
  { return pointInsideBoundaryRegion(boundary_region, point); };
  // Actual clipping algorithm
  const auto clipDualPolygonToBoundary =
      [&](const BoundaryRegion2D & boundary_region, const std::vector<Point> & dual_points)
  {
    if (boundary_region.boundary_clip_segments.empty())
      return dual_points;

    std::vector<Point> clipped_points;

    if (dual_points.size() < 3)
      return clipped_points;

    for (const auto & point : dual_points)
      if (pointInsideBoundary(boundary_region, point))
        addUniquePoint(clipped_points, point, length_tol);

    for (const auto i : index_range(dual_points))
    {
      const Point & p0 = dual_points[i];
      const Point & p1 = dual_points[(i + 1) % dual_points.size()];

      for (const auto & boundary_segment : boundary_region.boundary_clip_segments)
        addSegmentIntersections2D(clipped_points,
                                  p0,
                                  p1,
                                  boundary_segment.first,
                                  boundary_segment.second,
                                  length_tol,
                                  area_tol,
                                  parameter_tol);
    }

    for (const auto & boundary_segment : boundary_region.boundary_clip_segments)
    {
      if (pointInPolygon2D(boundary_segment.first, dual_points, length_tol, area_tol))
        addUniquePoint(clipped_points, boundary_segment.first, length_tol);

      if (pointInPolygon2D(boundary_segment.second, dual_points, length_tol, area_tol))
        addUniquePoint(clipped_points, boundary_segment.second, length_tol);
    }

    if (clipped_points.size() < 3)
      return clipped_points;

    std::vector<Point> unique_clipped_points;

    for (const auto & point : clipped_points)
      addUniquePoint(unique_clipped_points, point, length_tol);

    if (unique_clipped_points.size() > 1 &&
        MooseUtils::absoluteFuzzyEqual(
            unique_clipped_points.front(), unique_clipped_points.back(), length_tol))
      unique_clipped_points.pop_back();

    if (unique_clipped_points.size() < 3)
      return unique_clipped_points;

    std::vector<std::vector<std::size_t>> adjacency(unique_clipped_points.size());

    const auto addAdjacencyEdge = [&](const std::size_t point0_id, const std::size_t point1_id)
    {
      if (point0_id == point1_id)
        return;

      if (std::find(adjacency[point0_id].begin(), adjacency[point0_id].end(), point1_id) ==
          adjacency[point0_id].end())
        adjacency[point0_id].push_back(point1_id);

      if (std::find(adjacency[point1_id].begin(), adjacency[point1_id].end(), point0_id) ==
          adjacency[point1_id].end())
        adjacency[point1_id].push_back(point0_id);
    };

    const auto segmentParameter =
        [&](const Point & point, const Point & segment_start, const Point & segment_end)
    {
      const Point segment_direction = segment_end - segment_start;
      const Real segment_length_sq = segment_direction * segment_direction;

      if (segment_length_sq <= length_tol * length_tol)
        return Real(0.0);

      return ((point - segment_start) * segment_direction) / segment_length_sq;
    };

    const auto connectSegmentPoints = [&](const Point & segment_start,
                                          const Point & segment_end,
                                          const auto & midpointInsidePredicate)
    {
      std::vector<std::size_t> segment_point_ids;

      for (const auto point_id : index_range(unique_clipped_points))
        if (pointOnSegment2D(
                unique_clipped_points[point_id], segment_start, segment_end, length_tol, area_tol))
          segment_point_ids.push_back(point_id);

      if (segment_point_ids.size() < 2)
        return;

      std::sort(
          segment_point_ids.begin(),
          segment_point_ids.end(),
          [&](const std::size_t point0_id, const std::size_t point1_id)
          {
            return segmentParameter(unique_clipped_points[point0_id], segment_start, segment_end) <
                   segmentParameter(unique_clipped_points[point1_id], segment_start, segment_end);
          });

      for (const auto i : make_range(segment_point_ids.size() - 1))
      {
        const Point midpoint = 0.5 * (unique_clipped_points[segment_point_ids[i]] +
                                      unique_clipped_points[segment_point_ids[i + 1]]);

        if (midpointInsidePredicate(midpoint))
          addAdjacencyEdge(segment_point_ids[i], segment_point_ids[i + 1]);
      }
    };

    for (const auto i : index_range(dual_points))
      connectSegmentPoints(dual_points[i],
                           dual_points[(i + 1) % dual_points.size()],
                           [&](const Point & midpoint)
                           { return pointInsideBoundary(boundary_region, midpoint); });

    for (const auto & boundary_segment : boundary_region.boundary_clip_segments)
      connectSegmentPoints(
          boundary_segment.first,
          boundary_segment.second,
          [&](const Point & midpoint)
          { return pointInPolygon2D(midpoint, dual_points, length_tol, area_tol); });

    std::size_t start_point_id = unique_clipped_points.size();

    for (const auto point_id : index_range(unique_clipped_points))
      if (adjacency[point_id].size() >= 2 &&
          (start_point_id == unique_clipped_points.size() ||
           unique_clipped_points[point_id](0) < unique_clipped_points[start_point_id](0) ||
           (std::abs(unique_clipped_points[point_id](0) -
                     unique_clipped_points[start_point_id](0)) <= length_tol &&
            unique_clipped_points[point_id](1) < unique_clipped_points[start_point_id](1))))
        start_point_id = point_id;

    if (start_point_id == unique_clipped_points.size())
      return unique_clipped_points;

    std::vector<Point> best_ordered_points;

    for (const auto start_neighbor_id : adjacency[start_point_id])
    {
      std::vector<std::size_t> ordered_point_ids = {start_point_id};
      std::size_t previous_point_id = start_point_id;
      std::size_t current_point_id = start_neighbor_id;
      bool closed_cycle = false;

      while (ordered_point_ids.size() <= unique_clipped_points.size())
      {
        ordered_point_ids.push_back(current_point_id);

        const auto & current_adjacency = adjacency[current_point_id];
        std::size_t next_point_id = unique_clipped_points.size();

        for (const auto adjacent_point_id : current_adjacency)
          if (adjacent_point_id != previous_point_id)
          {
            next_point_id = adjacent_point_id;
            break;
          }

        if (next_point_id == start_point_id)
        {
          closed_cycle = true;
          break;
        }

        if (next_point_id == unique_clipped_points.size())
          break;

        previous_point_id = current_point_id;
        current_point_id = next_point_id;
      }

      if (!closed_cycle)
        continue;

      std::vector<Point> ordered_points;
      ordered_points.reserve(ordered_point_ids.size());

      for (const auto point_id : ordered_point_ids)
        ordered_points.push_back(unique_clipped_points[point_id]);

      if (best_ordered_points.empty() || ordered_points.size() > best_ordered_points.size())
        best_ordered_points = std::move(ordered_points);
    }

    if (!best_ordered_points.empty())
      return best_ordered_points;

    Point center;

    for (const auto & point : unique_clipped_points)
      center += point;

    center /= unique_clipped_points.size();

    std::sort(unique_clipped_points.begin(),
              unique_clipped_points.end(),
              [&center](const Point & a, const Point & b)
              {
                return std::atan2(a(1) - center(1), a(0) - center(0)) <
                       std::atan2(b(1) - center(1), b(0) - center(0));
              });

    return unique_clipped_points;
  };

  // Helper for finding if a node is a boundary vertex
  const auto isBoundaryVertexPoint =
      [&](const BoundaryRegion2D & boundary_region, const Point & point)
  {
    for (const auto boundary_vertex_node : boundary_region.boundary_vertex_nodes)
    {
      const auto point_it = boundary_region.boundary_node_points.find(boundary_vertex_node);

      if (point_it != boundary_region.boundary_node_points.end() &&
          MooseUtils::absoluteFuzzyEqual(point, point_it->second, length_tol))
        return true;
    }

    return false;
  };

  // Helper for finding if a node lies on a boundary segment, for our clipping routine
  const auto isBoundarySegmentPoint =
      [&](const BoundaryRegion2D & boundary_region, const Point & point)
  {
    for (const auto & boundary_segment : boundary_region.boundary_clip_segments)
      if (pointOnSegment2D(
              point, boundary_segment.first, boundary_segment.second, length_tol, area_tol))
        return true;

    return false;
  };
  // Concave element indexing helper, used for reordering concave dual cells.
  const auto concaveBoundaryVertexIndex = [&](const BoundaryRegion2D & boundary_region,
                                              const std::vector<Point> & points) -> std::size_t
  {
    if (points.size() < 4)
      return points.size();

    const Real signed_area = polygonSignedArea2D(points);

    if (std::abs(signed_area) < area_tol)
      return points.size();

    const Real orientation = signed_area > 0.0 ? 1.0 : -1.0;

    for (const auto i : index_range(points))
    {
      const Point & previous = points[(i + points.size() - 1) % points.size()];
      const Point & current = points[i];
      const Point & next = points[(i + 1) % points.size()];

      if (isBoundaryVertexPoint(boundary_region, current) &&
          orientation * cross2D(previous, current, next) < -area_tol)
        return i;
    }

    return points.size();
  };

  const auto addDualElement =
      [&](const std::vector<Point> & points, const SubdomainID output_subdomain_id)
  {
    auto buildDualElement = [&](const std::vector<Node *> & nodes)
    {
      auto dual_elem = std::make_unique<libMesh::C0Polygon>(nodes.size());

      for (const auto i : index_range(nodes))
        dual_elem->set_node(i, nodes[i]);

      return dual_elem;
    };
    std::vector<Node *> dual_nodes(points.size());

    for (const auto i : index_range(points))
      dual_nodes[i] = getDualNode(points[i]);

    auto dual_elem = buildDualElement(dual_nodes);

    // Fixing flipped elems
    if (dual_elem->is_flipped())
    {
      std::vector<Node *> reversed_nodes(points.size());
      reversed_nodes[0] = dual_nodes[0];

      for (const auto i : make_range(std::size_t(1), points.size()))
        reversed_nodes[i] = dual_nodes[points.size() - i];

      dual_elem = buildDualElement(reversed_nodes);
    }

    if (!dual_elem->is_flipped())
    {
      if (output_subdomain_id != Elem::invalid_subdomain_id)
        dual_elem->subdomain_id() = output_subdomain_id;
      dualMesh->add_elem(std::move(dual_elem));
    }
  };

  // Build one dual element around each source node.
  for (const auto & node_elems : source_node_to_elems)
  {
    const NodeSubdomainKey source_node_subdomain_key = node_elems.first;
    const dof_id_type source_node_id = source_node_subdomain_key.first;
    const SubdomainID source_subdomain_id = source_node_subdomain_key.second;
    const auto & incident_elems = node_elems.second;

    if (incident_elems.empty())
      continue;

    const auto boundary_region_it = boundary_regions.find(source_subdomain_id);
    const BoundaryRegion2D * const boundary_region =
        boundary_region_it != boundary_regions.end() ? &boundary_region_it->second : nullptr;

    std::vector<std::vector<unsigned int>> adjacency(incident_elems.size());

    for (const auto i : index_range(incident_elems))
      for (const auto j : make_range(i + 1, incident_elems.size()))
        if (elementsShareTwoNodes(incident_elems[i], incident_elems[j]))
        {
          adjacency[i].push_back(j);
          adjacency[j].push_back(i);
        }

    // Start at an endpoint for boundary chains, otherwise start anywhere.
    unsigned int start = 0;

    for (const auto i : index_range(adjacency))
      if (adjacency[i].size() == 1)
      {
        start = i;
        break;
      }

    std::vector<bool> used(incident_elems.size(), false);
    std::vector<dof_id_type> ordered_center_ids;

    unsigned int current = start;
    unsigned int previous = libMesh::invalid_uint;

    // Walk element adjacency to collect dual centers in connected order.
    while (true)
    {
      const Elem * elem = incident_elems[current];

      auto it = source_elem_to_center_id.find(elem->id());
      if (it != source_elem_to_center_id.end())
      {
        const dof_id_type center_id = it->second;

        if (std::find(ordered_center_ids.begin(), ordered_center_ids.end(), center_id) ==
            ordered_center_ids.end())
          ordered_center_ids.push_back(center_id);
      }

      used[current] = true;

      unsigned int next = libMesh::invalid_uint;

      for (const auto candidate : adjacency[current])
        if (candidate != previous && !used[candidate])
        {
          next = candidate;
          break;
        }

      if (next == libMesh::invalid_uint)
        break;

      previous = current;
      current = next;
    }

    std::vector<Point> dual_points;
    std::vector<Point> source_center_points;

    for (const auto center_id : ordered_center_ids)
    {
      addUniquePoint(dual_points, dual_centers[center_id], length_tol);
      source_center_points.push_back(dual_centers[center_id]);
    }

    if (!use_voronoi || _preserve_subdomain_interfaces)
    {
      if (boundary_region != nullptr)
      {
        const auto boundary_midpoint_it =
            boundary_region->boundary_node_midpoints.find(source_node_id);

        if (boundary_midpoint_it != boundary_region->boundary_node_midpoints.end())
        {
          const auto boundary_point_it = boundary_region->boundary_node_points.find(source_node_id);

          if (boundary_region->boundary_vertex_nodes.count(source_node_id) &&
              boundary_point_it != boundary_region->boundary_node_points.end())
            addUniquePoint(
                dual_points, boundary_point_it->second, length_tol); // Add primal vertices

          for (const auto & midpoint : boundary_midpoint_it->second)
            addUniquePoint(dual_points, midpoint, length_tol);

          if (boundary_point_it != boundary_region->boundary_node_points.end() &&
              !source_center_points.empty())
          {
            Point center_average; // Our source center is not quite the centroid, it's the midpoint
                                  // between the dual nodes' centroid and the primal boundary point.

            for (const auto & center_point : source_center_points)
              center_average += center_point;

            center_average /= source_center_points.size();

            const Point sort_center = 0.5 * (boundary_point_it->second + center_average);

            // Sorting is necessary, as the centroid ordering is jumbled after adding vertices and
            // midpoints.
            std::sort(dual_points.begin(),
                      dual_points.end(),
                      [&sort_center](const Point & a, const Point & b)
                      {
                        return std::atan2(a(1) - sort_center(1), a(0) - sort_center(0)) <
                               std::atan2(b(1) - sort_center(1), b(0) - sort_center(0));
                      });
          }
        }
      }
    }

    if (boundary_region != nullptr && dual_points.size() >= 3)
      // Finally, clipping back to boundary.
      dual_points = clipDualPolygonToBoundary(*boundary_region, dual_points);

    if (dual_points.size() < 3)
      continue;

    const std::size_t concave_vertex_index =
        boundary_region != nullptr ? concaveBoundaryVertexIndex(*boundary_region, dual_points)
                                   : dual_points.size();

    if (concave_vertex_index < dual_points.size())
    {
      const Point corner_point = dual_points[concave_vertex_index];
      std::vector<std::pair<Point, Real>> sorted_points;

      // We use phi sorting only for concave dual elements, since the id's are always too jumbled to
      // sort by adjacency
      for (const auto i : index_range(dual_points))
      {
        if (i == concave_vertex_index)
          continue;

        const Real phi =
            std::atan2(dual_points[i](1) - corner_point(1), dual_points[i](0) - corner_point(0));
        sorted_points.push_back({dual_points[i], phi});
      }

      std::sort(sorted_points.begin(),
                sorted_points.end(),
                [](const auto & a, const auto & b) { return a.second < b.second; });

      std::vector<Point> fan_points = {corner_point};

      for (const auto i : index_range(sorted_points))
      {
        if (boundary_region == nullptr ||
            !isBoundarySegmentPoint(*boundary_region, sorted_points[i].first))
          continue;

        const std::size_t next_i = (i + 1) % sorted_points.size();
        const std::size_t prev_i = (i + sorted_points.size() - 1) % sorted_points.size();

        // Pick the direction of the fan ordering so no triangle formed from the concave corner
        // bridges across a boundary.
        if (!isBoundarySegmentPoint(*boundary_region, sorted_points[next_i].first))
        {
          for (const auto k : index_range(sorted_points))
            fan_points.push_back(sorted_points[(i + k) % sorted_points.size()].first);

          break;
        }

        if (!isBoundarySegmentPoint(*boundary_region, sorted_points[prev_i].first))
        {
          for (const auto k : index_range(sorted_points))
            fan_points.push_back(
                sorted_points[(i + sorted_points.size() - k) % sorted_points.size()].first);

          break;
        }
      }

      if (!use_voronoi)
      {
        // Barycentric dual cells can be added as concave polygons, so only Voronoi dual cells need
        // the triangle fan below.
        if (fan_points.size() >= 3)
          addDualElement(fan_points, source_subdomain_id);

        continue;
      }

      if (fan_points.size() >= 3)
      {
        for (const auto i : make_range(std::size_t(1), fan_points.size() - 1))
        {
          const std::vector<Point> triangle_points = {
              fan_points[0], fan_points[i], fan_points[i + 1]};

          if (std::abs(cross2D(triangle_points[0], triangle_points[1], triangle_points[2])) >
              area_tol)
            addDualElement(triangle_points, source_subdomain_id);
        }
      }

      continue;
    }

    addDualElement(dual_points, source_subdomain_id);
  }

  dualMesh->unset_is_prepared();

  return dynamic_pointer_cast<MeshBase>(dualMesh);
}

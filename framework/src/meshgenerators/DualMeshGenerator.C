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
#include "MooseMeshUtils.h"
#include "libmesh/node_elem.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/elem.h"

#include <algorithm>
#include <cmath>

registerMooseObject("MooseApp", DualMeshGenerator);

InputParameters
DualMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  MooseEnum dual_mesh_type("voronoi barycentric", "barycentric");
  params.addParam<MooseEnum>(
      "dual_mesh_type",
      dual_mesh_type,
      "Whether to place dual nodes at Delaunay circumcenters or primal element centroids.");
  params.addRangeCheckedParam<Real>(
      "boundary_node_angular_tol",
      1e-8,
      "boundary_node_angular_tol>=0",
      "Angular tolerance, in radians, used to decide whether a primal boundary node is collinear "
      "with its two adjacent boundary edges. Nodes whose boundary angle differs from pi by more "
      "than this tolerance are treated as primal boundary vertices.");
  params.addClassDescription("Takes a 2D mesh as input and returns a dual mesh, i.e.,"
                             "changes each input node into an element and each input element "
                             "into a node located at its circumcenter or centroid.");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundary_node_angular_tol(getParam<Real>("boundary_node_angular_tol")),
    _dual_mesh_type(getParam<MooseEnum>("dual_mesh_type"))
{
}

// Circumcenter method
Point
DualMeshGenerator::circumcenter(const Elem * elem)
{
  const unsigned int n = elem->n_vertices();
  libmesh_assert_greater(n, 2);

  const Point & p0 = elem->point(0);

  Real A11 = 0.;
  Real A12 = 0.;
  Real A22 = 0.;

  Real b1 = 0.;
  Real b2 = 0.;

  for (unsigned int i = 1; i < n; ++i)
  {
    const Point & pi = elem->point(i);

    const Real dx = pi(0) - p0(0);
    const Real dy = pi(1) - p0(1);

    const Real rhs = 0.5 * (pi(0) * pi(0) + pi(1) * pi(1) - p0(0) * p0(0) - p0(1) * p0(1));

    A11 += dx * dx;
    A12 += dx * dy;
    A22 += dy * dy;

    b1 += dx * rhs;
    b2 += dy * rhs;
  }

  const Real det = A11 * A22 - A12 * A12;

  const Real cx = (A22 * b1 - A12 * b2) / det;
  const Real cy = (A11 * b2 - A12 * b1) / det;

  return Point(cx, cy, 0.0);
}

// True only when two elements share a full edge.
static bool
elementsShareTwoNodes(const Elem * a, const Elem * b)
{
  unsigned int shared_nodes = 0;

  for (unsigned int i = 0; i < a->n_nodes(); ++i)
    for (unsigned int j = 0; j < b->n_nodes(); ++j)
      if (a->node_id(i) == b->node_id(j))
        ++shared_nodes;

  return shared_nodes == 2;
}

static Real
cross2D(const Point & a, const Point & b, const Point & c)
{
  return (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0));
}

static bool
samePoint2D(const Point & a, const Point & b, const Real tol = 1e-12)
{
  return (a - b).norm() <= tol;
}

static void
addUniquePoint(std::vector<Point> & points, const Point & point, const Real tol = 1e-12)
{
  for (const auto & existing_point : points)
    if (samePoint2D(existing_point, point, tol))
      return;

  points.push_back(point);
}

static bool
pointOnSegment2D(const Point & point, const Point & a, const Point & b, const Real tol = 1e-12)
{
  if (std::abs(cross2D(a, b, point)) > tol)
    return false;

  return (point(0) >= std::min(a(0), b(0)) - tol && point(0) <= std::max(a(0), b(0)) + tol &&
          point(1) >= std::min(a(1), b(1)) - tol && point(1) <= std::max(a(1), b(1)) + tol);
}

static void
addSegmentIntersections2D(std::vector<Point> & points,
                          const Point & p0,
                          const Point & p1,
                          const Point & q0,
                          const Point & q1,
                          const Real tol = 1e-12)
{
  const Point r = p1 - p0;
  const Point s = q1 - q0;
  const Real denom = r(0) * s(1) - r(1) * s(0);

  if (std::abs(denom) < tol)
  {
    if (std::abs(cross2D(p0, p1, q0)) > tol)
      return;

    if (pointOnSegment2D(q0, p0, p1, tol))
      addUniquePoint(points, q0, tol);
    if (pointOnSegment2D(q1, p0, p1, tol))
      addUniquePoint(points, q1, tol);
    if (pointOnSegment2D(p0, q0, q1, tol))
      addUniquePoint(points, p0, tol);
    if (pointOnSegment2D(p1, q0, q1, tol))
      addUniquePoint(points, p1, tol);

    return;
  }

  const Point qp = q0 - p0;
  const Real t = (qp(0) * s(1) - qp(1) * s(0)) / denom;
  const Real u = (qp(0) * r(1) - qp(1) * r(0)) / denom;

  if (t >= -tol && t <= 1.0 + tol && u >= -tol && u <= 1.0 + tol)
    addUniquePoint(points, p0 + t * r, tol);
}

static bool
pointInPolygon2D(const Point & point, const std::vector<Point> & polygon, const Real tol = 1e-12)
{
  if (polygon.size() < 3)
    return false;

  bool inside = false;

  for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
  {
    const Point & pi = polygon[i];
    const Point & pj = polygon[j];

    if (pointOnSegment2D(point, pj, pi, tol))
      return true;

    if ((pi(1) > point(1)) != (pj(1) > point(1)) &&
        point(0) < (pj(0) - pi(0)) * (point(1) - pi(1)) / (pj(1) - pi(1)) + pi(0))
      inside = !inside;
  }

  return inside;
}

static Real
polygonSignedArea2D(const std::vector<Point> & polygon)
{
  if (polygon.size() < 3)
    return 0.0;

  Real area = 0.0;

  for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
    area += polygon[j](0) * polygon[i](1) - polygon[i](0) * polygon[j](1);

  return 0.5 * area;
}

struct BoundarySegment
{
  dof_id_type node0;
  dof_id_type node1;
  Point p0;
  Point p1;
};

std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  const auto input_mesh = std::move(_input);

  if (input_mesh->mesh_dimension() != 2)
    mooseError("DualMeshGenerator currently only supports 2D Meshes");

  const bool use_voronoi = _dual_mesh_type == "voronoi";

  std::unordered_map<dof_id_type, Point> boundary_node_points;
  std::unordered_map<dof_id_type, std::vector<Point>> boundary_node_midpoints;
  std::vector<BoundarySegment> physical_boundary_segments;

  for (const auto & elem : input_mesh->element_ptr_range())
  {
    for (const auto side : elem->side_index_range())
    {
      if (elem->neighbor_ptr(side) == nullptr)
      {
        auto side_elem = elem->build_side_ptr(side);

        if (side_elem->n_nodes() == 2)
        {
          const dof_id_type node0 = side_elem->node_id(0);
          const dof_id_type node1 = side_elem->node_id(1);

          physical_boundary_segments.push_back(
              {node0, node1, side_elem->point(0), side_elem->point(1)});

          boundary_node_points[node0] = side_elem->point(0);
          boundary_node_points[node1] = side_elem->point(1);

          const Point midpoint = 0.5 * (side_elem->point(0) + side_elem->point(1));
          boundary_node_midpoints[node0].push_back(midpoint);
          boundary_node_midpoints[node1].push_back(midpoint);
        }
      }
    }
  }

  std::unordered_map<dof_id_type, std::vector<std::size_t>> boundary_node_to_segments;

  for (std::size_t i = 0; i < physical_boundary_segments.size(); ++i)
  {
    boundary_node_to_segments[physical_boundary_segments[i].node0].push_back(i);
    boundary_node_to_segments[physical_boundary_segments[i].node1].push_back(i);
  }

  const auto otherBoundaryNode = [&](const BoundarySegment & segment,
                                     const dof_id_type node_id) -> dof_id_type
  { return segment.node0 == node_id ? segment.node1 : segment.node0; };

  std::unordered_set<dof_id_type> boundary_vertex_nodes;

  for (const auto & node_segments : boundary_node_to_segments)
  {
    const dof_id_type node_id = node_segments.first;
    const auto & segment_ids = node_segments.second;

    if (segment_ids.size() != 2)
    {
      boundary_vertex_nodes.insert(node_id);
      continue;
    }

    const Point & p = boundary_node_points[node_id];
    const Point v0 = boundary_node_points[otherBoundaryNode(
                         physical_boundary_segments[segment_ids[0]], node_id)] -
                     p;
    const Point v1 = boundary_node_points[otherBoundaryNode(
                         physical_boundary_segments[segment_ids[1]], node_id)] -
                     p;

    const Real norm_product = v0.norm() * v1.norm();

    if (norm_product < 1e-14)
    {
      boundary_vertex_nodes.insert(node_id);
      continue;
    }

    Real cos_angle = (v0 * v1) / norm_product;
    cos_angle = std::max(Real(-1.0), std::min(Real(1.0), cos_angle));

    if (std::abs(libMesh::pi - std::acos(cos_angle)) > _boundary_node_angular_tol)
      boundary_vertex_nodes.insert(node_id);
  }

  std::vector<std::pair<Point, Point>> boundary_clip_segments;
  std::vector<bool> used_boundary_segments(physical_boundary_segments.size(), false);

  const auto traceBoundaryClipSegment =
      [&](const dof_id_type start_node, const std::size_t start_segment_id)
  {
    dof_id_type current_node = start_node;
    std::size_t current_segment_id = start_segment_id;

    while (current_segment_id < physical_boundary_segments.size() &&
           !used_boundary_segments[current_segment_id])
    {
      used_boundary_segments[current_segment_id] = true;

      const auto & segment = physical_boundary_segments[current_segment_id];
      const dof_id_type next_node = otherBoundaryNode(segment, current_node);

      if (next_node == start_node || boundary_vertex_nodes.count(next_node))
      {
        boundary_clip_segments.push_back(
            {boundary_node_points[start_node], boundary_node_points[next_node]});
        return;
      }

      const auto node_segment_it = boundary_node_to_segments.find(next_node);

      if (node_segment_it == boundary_node_to_segments.end())
        return;

      std::size_t next_segment_id = physical_boundary_segments.size();

      for (const auto candidate_segment_id : node_segment_it->second)
        if (!used_boundary_segments[candidate_segment_id])
        {
          next_segment_id = candidate_segment_id;
          break;
        }

      current_node = next_node;
      current_segment_id = next_segment_id;
    }
  };

  for (const auto boundary_vertex_node : boundary_vertex_nodes)
  {
    const auto node_segment_it = boundary_node_to_segments.find(boundary_vertex_node);

    if (node_segment_it == boundary_node_to_segments.end())
      continue;

    for (const auto segment_id : node_segment_it->second)
      if (!used_boundary_segments[segment_id])
        traceBoundaryClipSegment(boundary_vertex_node, segment_id);
  }

  for (std::size_t segment_id = 0; segment_id < physical_boundary_segments.size(); ++segment_id)
    if (!used_boundary_segments[segment_id])
      traceBoundaryClipSegment(physical_boundary_segments[segment_id].node0, segment_id);

  std::vector<Point> dual_centers;
  std::unordered_map<dof_id_type, dof_id_type> source_elem_to_center_id;
  std::unordered_map<dof_id_type, std::vector<const Elem *>> source_node_to_elems;
  std::unique_ptr<ReplicatedMesh> tri_mesh;

  if (use_voronoi)
  {
    tri_mesh = buildReplicatedMesh(2);

    for (const auto & node : input_mesh->node_ptr_range())
    {
      Node * new_node = tri_mesh->add_point(*node);

      auto node_elem = std::make_unique<NodeElem>();
      node_elem->set_node(0) = new_node;
      tri_mesh->add_elem(std::move(node_elem));
    }

    Node * p0 = tri_mesh->add_point(Point(-100.0, -100.0, 0.0));
    Node * p1 = tri_mesh->add_point(Point(100.0, -100.0, 0.0));
    Node * p2 = tri_mesh->add_point(Point(100.0, 100.0, 0.0));
    Node * p3 = tri_mesh->add_point(Point(-100.0, 100.0, 0.0));

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

    triangulator.insert_extra_points() = false;
    triangulator.smooth_after_generating() = false;

    triangulator.triangulate();

    for (const auto & tri_elem : tri_mesh->element_ptr_range())
    {
      if (tri_elem->n_vertices() != 3)
        continue;

      const dof_id_type center_id = dual_centers.size();

      dual_centers.push_back(circumcenter(tri_elem));
      source_elem_to_center_id[tri_elem->id()] = center_id;

      for (const auto n : make_range(tri_elem->n_nodes()))
        source_node_to_elems[tri_elem->node_id(n)].push_back(tri_elem);
    }
  }
  else
  {
    for (const auto & elem : input_mesh->element_ptr_range())
    {
      const dof_id_type center_id = dual_centers.size();

      dual_centers.push_back(elem->true_centroid());
      source_elem_to_center_id[elem->id()] = center_id;

      for (const auto n : make_range(elem->n_nodes()))
        source_node_to_elems[elem->node_id(n)].push_back(elem);
    }
  }

  auto dualMesh = buildReplicatedMesh(2);

  const auto pointInsideBoundary = [&](const Point & point)
  {
    bool inside = false;

    for (const auto & segment : physical_boundary_segments)
    {
      if (pointOnSegment2D(point, segment.p0, segment.p1))
        return true;

      if ((segment.p0(1) > point(1)) != (segment.p1(1) > point(1)))
      {
        const Real intersection_x = (segment.p1(0) - segment.p0(0)) * (point(1) - segment.p0(1)) /
                                        (segment.p1(1) - segment.p0(1)) +
                                    segment.p0(0);

        if (point(0) < intersection_x)
          inside = !inside;
      }
    }

    return inside;
  };

  const auto clipDualPolygonToBoundary = [&](const std::vector<Point> & dual_points)
  {
    std::vector<Point> clipped_points;

    if (dual_points.size() < 3)
      return clipped_points;

    for (const auto & point : dual_points)
      if (pointInsideBoundary(point))
        addUniquePoint(clipped_points, point);

    for (unsigned int i = 0; i < dual_points.size(); ++i)
    {
      const Point & p0 = dual_points[i];
      const Point & p1 = dual_points[(i + 1) % dual_points.size()];

      for (const auto & boundary_segment : boundary_clip_segments)
        addSegmentIntersections2D(
            clipped_points, p0, p1, boundary_segment.first, boundary_segment.second);
    }

    for (const auto & boundary_segment : boundary_clip_segments)
    {
      if (pointInPolygon2D(boundary_segment.first, dual_points))
        addUniquePoint(clipped_points, boundary_segment.first);

      if (pointInPolygon2D(boundary_segment.second, dual_points))
        addUniquePoint(clipped_points, boundary_segment.second);
    }

    if (clipped_points.size() < 3)
      return clipped_points;

    Point center;

    for (const auto & point : clipped_points)
      center += point;

    center /= clipped_points.size();

    std::sort(clipped_points.begin(),
              clipped_points.end(),
              [&center](const Point & a, const Point & b)
              {
                return std::atan2(a(1) - center(1), a(0) - center(0)) <
                       std::atan2(b(1) - center(1), b(0) - center(0));
              });

    std::vector<Point> unique_clipped_points;

    for (const auto & point : clipped_points)
      addUniquePoint(unique_clipped_points, point);

    if (unique_clipped_points.size() > 1 &&
        samePoint2D(unique_clipped_points.front(), unique_clipped_points.back()))
      unique_clipped_points.pop_back();

    return unique_clipped_points;
  };

  const auto isBoundaryVertexPoint = [&](const Point & point)
  {
    for (const auto boundary_vertex_node : boundary_vertex_nodes)
    {
      const auto point_it = boundary_node_points.find(boundary_vertex_node);

      if (point_it != boundary_node_points.end() && samePoint2D(point, point_it->second))
        return true;
    }

    return false;
  };

  const auto isBoundarySegmentPoint = [&](const Point & point)
  {
    for (const auto & boundary_segment : boundary_clip_segments)
      if (pointOnSegment2D(point, boundary_segment.first, boundary_segment.second))
        return true;

    return false;
  };

  const auto concaveBoundaryVertexIndex = [&](const std::vector<Point> & points) -> std::size_t
  {
    if (points.size() < 4)
      return points.size();

    const Real signed_area = polygonSignedArea2D(points);

    if (std::abs(signed_area) < 1e-14)
      return points.size();

    const Real orientation = signed_area > 0.0 ? 1.0 : -1.0;

    for (std::size_t i = 0; i < points.size(); ++i)
    {
      const Point & previous = points[(i + points.size() - 1) % points.size()];
      const Point & current = points[i];
      const Point & next = points[(i + 1) % points.size()];

      if (isBoundaryVertexPoint(current) && orientation * cross2D(previous, current, next) < -1e-12)
        return i;
    }

    return points.size();
  };

  const auto addDualElement = [&](const std::vector<Point> & points)
  {
    auto dual_elem = std::make_unique<libMesh::C0Polygon>(points.size());

    for (unsigned int i = 0; i < points.size(); ++i)
      dual_elem->set_node(i, dualMesh->add_point(points[i]));

    // Fixing flipped elems
    if (dual_elem->is_flipped())
    {
      auto reversed_elem = std::make_unique<libMesh::C0Polygon>(points.size());

      for (unsigned int i = 0; i < points.size(); ++i)
        reversed_elem->set_node(i, dualMesh->add_point(points[points.size() - 1 - i]));

      dual_elem = std::move(reversed_elem);
    }

    if (!dual_elem->is_flipped())
      dualMesh->add_elem(std::move(dual_elem));
  };

  // Build one dual element around each source node.
  for (const auto & node_elems : source_node_to_elems)
  {
    const dof_id_type source_node_id = node_elems.first;
    const auto & incident_elems = node_elems.second;

    std::vector<std::vector<unsigned int>> adjacency(incident_elems.size());

    for (unsigned int i = 0; i < incident_elems.size(); ++i)
      for (unsigned int j = i + 1; j < incident_elems.size(); ++j)
        if (elementsShareTwoNodes(incident_elems[i], incident_elems[j]))
        {
          adjacency[i].push_back(j);
          adjacency[j].push_back(i);
        }

    // Start at an endpoint for boundary chains, otherwise start anywhere.
    unsigned int start = 0;

    for (unsigned int i = 0; i < adjacency.size(); ++i)
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
      addUniquePoint(dual_points, dual_centers[center_id]);
      source_center_points.push_back(dual_centers[center_id]);
    }

    if (!use_voronoi)
    {
      const auto boundary_midpoint_it = boundary_node_midpoints.find(source_node_id);

      if (boundary_midpoint_it != boundary_node_midpoints.end())
      {
        const auto boundary_point_it = boundary_node_points.find(source_node_id);

        if (boundary_vertex_nodes.count(source_node_id) &&
            boundary_point_it != boundary_node_points.end())
          addUniquePoint(dual_points, boundary_point_it->second);

        for (const auto & midpoint : boundary_midpoint_it->second)
          addUniquePoint(dual_points, midpoint);

        if (boundary_point_it != boundary_node_points.end() && !source_center_points.empty())
        {
          Point center_average;

          for (const auto & center_point : source_center_points)
            center_average += center_point;

          center_average /= source_center_points.size();

          const Point sort_center = 0.5 * (boundary_point_it->second + center_average);

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

    if (dual_points.size() >= 3)
      dual_points = clipDualPolygonToBoundary(dual_points);

    if (dual_points.size() < 3)
      continue;

    const bool split_concave_boundary_polygon = !use_voronoi;
    const std::size_t concave_vertex_index = split_concave_boundary_polygon
                                                 ? concaveBoundaryVertexIndex(dual_points)
                                                 : dual_points.size();

    if (concave_vertex_index < dual_points.size())
    {
      const Point corner_point = dual_points[concave_vertex_index];
      std::vector<std::pair<Point, Real>> sorted_points;

      // We use phi sorting only for concave dual elements, since the id's are always too jumbled to
      // sort by adjacency
      for (std::size_t i = 0; i < dual_points.size(); ++i)
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

      for (std::size_t i = 0; i < sorted_points.size(); ++i)
      {
        if (!isBoundarySegmentPoint(sorted_points[i].first))
          continue;

        const std::size_t next_i = (i + 1) % sorted_points.size();
        const std::size_t prev_i = (i + sorted_points.size() - 1) % sorted_points.size();

        // We pick the direction of fan-triangulating concave polygons such that we never create a
        // triangle that bridges across a boundary
        if (!isBoundarySegmentPoint(sorted_points[next_i].first))
        {
          for (std::size_t k = 0; k < sorted_points.size(); ++k)
            fan_points.push_back(sorted_points[(i + k) % sorted_points.size()].first);

          break;
        }

        if (!isBoundarySegmentPoint(sorted_points[prev_i].first))
        {
          for (std::size_t k = 0; k < sorted_points.size(); ++k)
            fan_points.push_back(
                sorted_points[(i + sorted_points.size() - k) % sorted_points.size()].first);

          break;
        }
      }

      if (fan_points.size() == 1)
        mooseError("Could not find a boundary point to start concave fan triangulation.");

      for (std::size_t i = 1; i + 1 < fan_points.size(); ++i)
      {
        const std::vector<Point> triangle_points = {
            fan_points[0], fan_points[i], fan_points[i + 1]};

        if (std::abs(cross2D(triangle_points[0], triangle_points[1], triangle_points[2])) > 1e-12)
          addDualElement(triangle_points);
      }

      continue;
    }

    addDualElement(dual_points);
  }

  dualMesh->unset_is_prepared();

  return dynamic_pointer_cast<MeshBase>(dualMesh);
}
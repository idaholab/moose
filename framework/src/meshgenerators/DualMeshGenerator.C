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
#include "libmesh/cell_c0polyhedron.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/libmesh_exceptions.h"
#include "libmesh/mesh_netgen_interface.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/node_elem.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/type_vector.h"
#include "libmesh/elem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
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
  MultiMooseEnum concave_treatment("netgen polycut split", "split polycut netgen");
  params.addParam<MultiMooseEnum>(
      "concave_treatment",
      concave_treatment,
      "Ordered treatments to attempt for concave 3D dual cells. Split attempts to split the "
      "candidate polyhedron by adding midpoint vertices, polycut attempts to split it into convex "
      "polyhedra using existing vertices, and netgen tetrahedralizes it, adding additional nodes "
      "when necessary.");
  params.addParam<bool>(
      "preserve_diagonals",
      false,
      "Whether candidate NetGen surface diagonals must remain inside the primal boundary, checked "
      "at their quarter, midpoint, and three-quarter points. When false, surface triangulation "
      "accepts the default highest-node-ID diagonal construction without boundary sampling.");
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
  params.addClassDescription("Takes a 2D or 3D mesh as input and returns a dual mesh, i.e., "
                             "changes each input node into an element and each input element "
                             "into a node located at its Delaunay triangulation's circumcenter "
                             "or its centroid.");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _dual_mesh_type(getParam<MooseEnum>("dual_mesh_type")),
    _concave_treatment(getParam<MultiMooseEnum>("concave_treatment")),
    _preserve_diagonals(getParam<bool>("preserve_diagonals")),
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
  std::vector<boundary_id_type> boundary_ids;
};

// Helps maps treat an edge the same regardless of direction
static std::pair<dof_id_type, dof_id_type>
edgeKey(const dof_id_type node0, const dof_id_type node1)
{
  return {std::min(node0, node1), std::max(node0, node1)};
}

// Used to reject degenerate polygon faces
static bool
hasNonzeroArea3D(const std::vector<Point> & points, const Real tol = 1e-12)
{
  for (const auto i : index_range(points))
    for (const auto j : make_range(i + 1, points.size()))
      for (const auto k : make_range(j + 1, points.size()))
        if (libMesh::cross_norm(points[j] - points[i], points[k] - points[i]) > tol)
          return true;

  return false;
}

// For adding polygon faces to polyhedrons
static void
addSidePoints3D(std::vector<std::vector<Point>> & sides,
                const std::vector<Point> & side_points,
                const Real tol = 1e-12)
{
  std::vector<Point> unique_side_points;

  for (const auto & point : side_points)
    addUniquePoint(unique_side_points, point, tol);

  if (unique_side_points.size() < 3 || !hasNonzeroArea3D(unique_side_points, tol))
    return;

  sides.push_back(unique_side_points);
}

// For finding a normal of the first non-collinear triple in a face's points
static Point
faceNormal3D(const std::vector<Point> & points, const Real tol = 1e-12)
{
  for (const auto i : index_range(points))
    for (const auto j : make_range(i + 1, points.size()))
      for (const auto k : make_range(j + 1, points.size()))
      {
        const Point normal = (points[j] - points[i]).cross(points[k] - points[i]);

        if (normal.norm() > tol)
          return normal;
      }

  return Point();
}

// Next few ...UnitDirection3D helpers are for surface normal tests to determine concavity of
// polyhedrons.
template <typename Range, typename GetDirection>
static bool
uniqueUnitDirection3D(const Range & directions,
                      const Point & direction,
                      Point & unit_direction,
                      const GetDirection & get_direction,
                      const Real tol = 1e-8)
{
  if (direction.norm() <= tol)
    return false;

  unit_direction = direction / direction.norm();

  for (const auto & existing_direction : directions)
    if ((unit_direction - get_direction(existing_direction)).norm() <= tol)
      return false;

  return true;
}

static bool
addUniqueUnitDirection3D(std::vector<Point> & directions,
                         const Point & direction,
                         const Real tol = 1e-8)
{
  Point unit_direction;

  if (!uniqueUnitDirection3D(
          directions,
          direction,
          unit_direction,
          [](const Point & point) -> const Point & { return point; },
          tol))
    return false;

  directions.push_back(unit_direction);
  return true;
}

// Used to get surface norms; required for surface normal concavity test
static void
addUniqueDirection3D(std::vector<Point> & directions,
                     const Point & direction,
                     const Real tol = 1e-8)
{
  addUniqueUnitDirection3D(directions, direction, tol);
}

struct BoundaryFaceNormal3D
{
  Point normal;
  Point face_centroid;
};

static void
addUniqueBoundaryFaceNormal3D(std::vector<BoundaryFaceNormal3D> & face_normals,
                              const Point & normal,
                              const Point & face_centroid,
                              const Real tol = 1e-8)
{
  Point unit_normal;

  if (!uniqueUnitDirection3D(
          face_normals,
          normal,
          unit_normal,
          [](const BoundaryFaceNormal3D & face_normal) -> const Point &
          { return face_normal.normal; },
          tol))
    return;

  face_normals.push_back({unit_normal, face_centroid});
}

using PointKey3D = std::array<Real, 3>;
using SegmentKey3D = std::pair<PointKey3D, PointKey3D>;

enum class DualPointSource3D
{
  primal_vertex,
  body_centroid,
  face_centroid,
  face_diagonal_midpoint,
  boundary_edge_midpoint
};

using DualPointSources3D = std::map<PointKey3D, std::set<DualPointSource3D>>;

struct ConnectedFacePoints3D
{
  std::vector<Point> points;
  std::vector<std::pair<Point, Point>> segments;
};

static PointKey3D
pointKey3D(const Point & point)
{
  return {{point(0), point(1), point(2)}};
}

static void
addDualPointSource3D(DualPointSources3D & point_sources,
                     const Point & point,
                     const DualPointSource3D source)
{
  point_sources[pointKey3D(point)].insert(source);
}

static const std::set<DualPointSource3D> *
findDualPointSources3D(const DualPointSources3D & point_sources, const Point & point)
{
  const auto source_it = point_sources.find(pointKey3D(point));
  return source_it == point_sources.end() ? nullptr : &source_it->second;
}

// Lets us refer to a set of two points as a segment regardless of orientation
static SegmentKey3D
segmentKey3D(const Point & point0, const Point & point1)
{
  auto key0 = pointKey3D(point0);
  auto key1 = pointKey3D(point1);

  if (key1 < key0)
    std::swap(key0, key1);

  return {key0, key1};
}

// Adds points to polyhedron input
static void
addConnectedFacePoint3D(ConnectedFacePoints3D & face_points,
                        const Point & point,
                        const Real tol = 1e-12)
{
  addUniquePoint(face_points.points, point, tol);
}

// Adds edges to polyhedrons, checking for zero-length segments
static void
addConnectedFaceSegment3D(ConnectedFacePoints3D & face_points,
                          const Point & point0,
                          const Point & point1,
                          const Real tol = 1e-12)
{
  if (MooseUtils::absoluteFuzzyEqual(point0, point1, tol))
    return;

  addConnectedFacePoint3D(face_points, point0, tol);
  addConnectedFacePoint3D(face_points, point1, tol);

  for (const auto & segment : face_points.segments)
    if ((MooseUtils::absoluteFuzzyEqual(segment.first, point0, tol) &&
         MooseUtils::absoluteFuzzyEqual(segment.second, point1, tol)) ||
        (MooseUtils::absoluteFuzzyEqual(segment.first, point1, tol) &&
         MooseUtils::absoluteFuzzyEqual(segment.second, point0, tol)))
      return;

  face_points.segments.push_back({point0, point1});
}

// Sorting, but 3D. Fallback for sortConnectedFacePoints3D since adding nodes can jumble
// connectivity ordering
static std::vector<Point>
sortPointsAroundAxis3D(const std::vector<Point> & unsorted_points,
                       const Point & axis,
                       const Real tol = 1e-12)
{
  std::vector<Point> points;

  for (const auto & point : unsorted_points)
    addUniquePoint(points, point, tol);

  if (points.size() < 3 || axis.norm() <= tol)
    return points;

  const Point axis_unit = axis / axis.norm();
  Point reference = std::abs(axis_unit(0)) < 0.9 ? Point(1.0, 0.0, 0.0) : Point(0.0, 1.0, 0.0);
  Point e1 = reference - (reference * axis_unit) * axis_unit;

  if (e1.norm() <= tol)
  {
    reference = Point(0.0, 0.0, 1.0);
    e1 = reference - (reference * axis_unit) * axis_unit;
  }

  if (e1.norm() <= tol)
    return points;

  e1 /= e1.norm();

  Point e2 = axis_unit.cross(e1);

  if (e2.norm() <= tol)
    return points;

  e2 /= e2.norm();

  Point center;

  for (const auto & point : points)
    center += point;

  center /= points.size();

  std::sort(points.begin(),
            points.end(),
            [&center, &e1, &e2](const Point & a, const Point & b)
            {
              const Point da = a - center;
              const Point db = b - center;

              return std::atan2(da * e2, da * e1) < std::atan2(db * e2, db * e1);
            });

  if (axis * faceNormal3D(points) < 0.0)
    std::reverse(points.begin(), points.end());

  return points;
}

// Face sorting to prevent tangling etc
static std::vector<Point>
sortConnectedFacePoints3D(const ConnectedFacePoints3D & unsorted_face_points,
                          const Point & axis,
                          const Real tol = 1e-12)
{
  std::vector<Point> points;

  for (const auto & point : unsorted_face_points.points)
    addUniquePoint(points, point, tol);

  if (points.size() < 3)
    return points;

  std::vector<std::vector<std::size_t>> point_neighbors(points.size());

  const auto pointIndex = [&](const Point & point) -> std::size_t
  {
    for (const auto i : index_range(points))
      if (MooseUtils::absoluteFuzzyEqual(points[i], point, tol))
        return i;

    mooseError("Could not locate connected face point while ordering 3D dual face.");
  };

  const auto addNeighbor = [&](const std::size_t point_index, const std::size_t neighbor_index)
  {
    for (const auto existing_neighbor_index : point_neighbors[point_index])
      if (existing_neighbor_index == neighbor_index)
        return;

    point_neighbors[point_index].push_back(neighbor_index);
  };

  for (const auto & segment : unsorted_face_points.segments)
  {
    const std::size_t point0_index = pointIndex(segment.first);
    const std::size_t point1_index = pointIndex(segment.second);

    if (point0_index == point1_index)
      continue;

    addNeighbor(point0_index, point1_index);
    addNeighbor(point1_index, point0_index);
  }

  for (const auto & neighbors : point_neighbors)
    if (neighbors.size() != 2)
      return sortPointsAroundAxis3D(points, axis, tol);

  std::vector<Point> ordered_points;
  std::vector<bool> visited(points.size(), false);
  std::size_t previous_index = points.size();
  std::size_t current_index = 0;

  for ([[maybe_unused]] const auto i : index_range(points))
  {
    if (visited[current_index])
      return sortPointsAroundAxis3D(points, axis, tol);

    ordered_points.push_back(points[current_index]);
    visited[current_index] = true;

    const auto & neighbors = point_neighbors[current_index];
    const std::size_t next_index = neighbors[0] == previous_index ? neighbors[1] : neighbors[0];

    previous_index = current_index;
    current_index = next_index;
  }

  if (current_index != 0)
    return sortPointsAroundAxis3D(points, axis, tol);

  if (axis * faceNormal3D(ordered_points) < 0.0)
    std::reverse(ordered_points.begin(), ordered_points.end());

  return ordered_points;
}

// Edge to be cut
struct PolyCutEdge3D
{
  Point p0;
  Point p1;
};

struct PolyCutResult3D
{
  std::vector<std::vector<Point>> child0_side_points;
  std::vector<std::vector<Point>> child1_side_points;
};

struct SplitEdgePoint3D
{
  Point edge_point0;
  Point edge_point1;
  Point split_point;
};

// Child faces, produced in polycut splitting
struct SplitFaceReplacement3D
{
  std::array<Point, 4> original_face_points;
  std::array<Point, 4> child0_face_points;
  std::array<Point, 4> child1_face_points;
};

// Split runs through candidate point pairs and sees which ones match the required criteria to
// produce viable child polyhedra.
struct SplitCutFaceCandidate3D
{
  std::vector<Point> cut_face;
  std::vector<SplitEdgePoint3D> split_edge_points;
  std::vector<SplitFaceReplacement3D> split_face_replacements;
  std::vector<std::vector<Point>> child0_side_points;
  std::vector<std::vector<Point>> child1_side_points;
};

// A primal side face plus any diagonal-specific dual point chosen for concave or twisted QUAD4
// faces.
struct SideFacePart3D
{
  std::vector<dof_id_type> node_ids;
  std::vector<Point> points;
  bool has_dual_point = false;
  Point dual_point;
  bool has_selected_diagonal = false;
  std::pair<dof_id_type, dof_id_type> selected_diagonal;
};

// Face centroids
static Point
centroid3D(const std::vector<Point> & points)
{
  Point centroid;

  for (const auto & point : points)
    centroid += point;

  if (!points.empty())
    centroid /= points.size();

  return centroid;
}

// Most side faces use their centroid, but a selected QUAD4 diagonal uses its midpoint to preserve
// the primal split surface.
static Point
sideFacePartDualPoint3D(const SideFacePart3D & side_face_part)
{
  if (side_face_part.has_dual_point)
    return side_face_part.dual_point;

  return centroid3D(side_face_part.points);
}

// Remember, in 3d, we have both face-centroids and body-centroids
static Point
polyhedronCentroid3D(const std::vector<std::vector<Point>> & side_points, const Real tol = 1e-12)
{
  std::vector<Point> unique_points;

  for (const auto & side : side_points)
    for (const auto & point : side)
      addUniquePoint(unique_points, point, tol);

  return centroid3D(unique_points);
}

// Measure how far a side face deviates from the first valid plane through three of its points.
// This catches twisted QUAD4 faces that need a deterministic triangulation.
static Real
sideFaceNonPlanarity3D(const std::vector<Point> & points, const Real length_tol)
{
  if (points.size() <= 3)
    return 0.0;

  const Real area_tol = length_tol * length_tol;

  for (const auto i : index_range(points))
    for (const auto j : make_range(i + 1, points.size()))
      for (const auto k : make_range(j + 1, points.size()))
      {
        Point normal = (points[j] - points[i]).cross(points[k] - points[i]);

        if (normal.norm() <= area_tol)
          continue;

        normal /= normal.norm();

        Real max_distance = 0.0;

        for (const auto & point : points)
          max_distance = std::max(max_distance, std::abs(normal * (point - points[i])));

        return max_distance;
      }

  return 0.0;
}

// Treat only faces that are visibly off-plane, relative to the scaled geometry tolerance, as
// non-planar.
static bool
sideFaceIsNonPlanar3D(const std::vector<Point> & points, const Real length_tol)
{
  if (points.size() <= 3)
    return false;

  return sideFaceNonPlanarity3D(points, length_tol) > length_tol;
}

// Match libMesh's 3D all_tri() rule for HEX8 side faces: split the QUAD4 through the highest
// global node id and its opposite node. This keeps dual faces aligned with an upstream
// ElementsToSimplicesConverter.
static bool
quad4PrimalSplitDiagonal3D(const std::vector<dof_id_type> & node_ids,
                           std::pair<dof_id_type, dof_id_type> & selected_diagonal)
{
  if (node_ids.size() != 4)
    return false;

  std::size_t highest_node_index = 0;

  for (const auto i : make_range(std::size_t(1), node_ids.size()))
    if (node_ids[i] > node_ids[highest_node_index])
      highest_node_index = i;

  const auto opposite_index = (highest_node_index + 2) % node_ids.size();
  selected_diagonal = edgeKey(node_ids[highest_node_index], node_ids[opposite_index]);
  return true;
}

// Prepare a primal side face for dual construction. Ordinary faces use their centroid as the dual
// point. Non-planar QUAD4 faces use the primal simplex-conversion diagonal so the dual preserves
// the primal triangulation.
static std::vector<SideFacePart3D>
sideFaceParts3D(const std::vector<dof_id_type> & node_ids,
                const std::vector<Point> & points,
                const Real length_tol,
                const std::pair<dof_id_type, dof_id_type> * const primal_split_diagonal = nullptr)
{
  mooseAssert(node_ids.size() == points.size(),
              "Primal side face node ids and points must have matching sizes.");

  SideFacePart3D side_face_part;
  side_face_part.node_ids = node_ids;
  side_face_part.points = points;

  if (points.size() != 4)
    return std::vector<SideFacePart3D>{side_face_part};

  // There are cases where we will have a primal mesh that is *sort of* concave. That is, it will
  // have a face element where not all of the points lie in a plane. We need to make sure we
  // preserve this geometry, and to do so we need to recover which diagonal to use. We follow the
  // convention of highest node ID
  const auto useSelectedDiagonal =
      [&](const std::pair<dof_id_type, dof_id_type> & selected_diagonal)
  {
    std::array<std::size_t, 2> diagonal_indices = {{points.size(), points.size()}};

    for (const auto i : index_range(node_ids))
      if (node_ids[i] == selected_diagonal.first)
        diagonal_indices[0] = i;
      else if (node_ids[i] == selected_diagonal.second)
        diagonal_indices[1] = i;

    if (diagonal_indices[0] == points.size() || diagonal_indices[1] == points.size())
      mooseError("Selected primal QUAD4 diagonal nodes were not found in the side face.");

    if ((diagonal_indices[0] + 2) % points.size() != diagonal_indices[1] &&
        (diagonal_indices[1] + 2) % points.size() != diagonal_indices[0])
      mooseError("Selected primal QUAD4 diagonal is not between opposite vertices.");

    side_face_part.has_selected_diagonal = true;
    side_face_part.selected_diagonal = selected_diagonal;
    side_face_part.has_dual_point = true;
    side_face_part.dual_point = 0.5 * (points[diagonal_indices[0]] + points[diagonal_indices[1]]);
  };

  if (sideFaceIsNonPlanar3D(points, length_tol))
  {
    std::pair<dof_id_type, dof_id_type> selected_diagonal;

    if (primal_split_diagonal)
      selected_diagonal = *primal_split_diagonal;
    else if (!quad4PrimalSplitDiagonal3D(node_ids, selected_diagonal))
      mooseError("Could not determine primal QUAD4 split diagonal.");

    useSelectedDiagonal(selected_diagonal);
  }

  return std::vector<SideFacePart3D>{side_face_part};
}

static std::vector<std::array<std::size_t, 3>>
sideFacePartTriangleIndices3D(const SideFacePart3D & side_face_part, const Real length_tol)
{
  const auto & node_ids = side_face_part.node_ids;
  const auto & points = side_face_part.points;
  std::vector<std::array<std::size_t, 3>> triangle_indices;

  if (points.size() < 3)
    return triangle_indices;

  // Adds nondegenerate (usually, nonzero-area) triangles
  const auto addTriangle = [&](const std::array<std::size_t, 3> & point_indices)
  {
    std::vector<Point> triangle = {
        points[point_indices[0]], points[point_indices[1]], points[point_indices[2]]};

    if (hasNonzeroArea3D(triangle, length_tol * length_tol))
      triangle_indices.push_back(point_indices);
  };

  if (points.size() == 4 && side_face_part.has_selected_diagonal)
  {
    std::array<std::size_t, 2> diagonal_indices = {{points.size(), points.size()}};

    for (const auto i : index_range(node_ids))
      if (node_ids[i] == side_face_part.selected_diagonal.first)
        diagonal_indices[0] = i;
      else if (node_ids[i] == side_face_part.selected_diagonal.second)
        diagonal_indices[1] = i;

    if (diagonal_indices[0] == points.size() || diagonal_indices[1] == points.size())
      mooseError("Selected primal QUAD4 diagonal nodes were not found in the side face.");

    if ((diagonal_indices[0] + 2) % points.size() != diagonal_indices[1] &&
        (diagonal_indices[1] + 2) % points.size() != diagonal_indices[0])
      mooseError("Selected primal QUAD4 diagonal is not between opposite vertices.");

    if ((diagonal_indices[0] == 0 && diagonal_indices[1] == 2) ||
        (diagonal_indices[0] == 2 && diagonal_indices[1] == 0))
    {
      addTriangle({{0, 1, 2}});
      addTriangle({{0, 2, 3}});
    }
    else
    {
      addTriangle({{1, 2, 3}});
      addTriangle({{1, 3, 0}});
    }

    return triangle_indices;
  }

  for (const auto n : make_range(std::size_t(1), points.size() - 1))
    addTriangle({{0, n, n + 1}});

  return triangle_indices;
}

// Triangulate a side-face part with the selected primal diagonal when one is available. NetGen and
// point-in-polyhedron checks both consume these triangles, so diagonal consistency controls whether
// adjacent dual cells share the same surface.
static std::vector<std::vector<Point>>
sideFacePartTriangles3D(const SideFacePart3D & side_face_part, const Real length_tol)
{
  std::vector<std::vector<Point>> triangles;

  for (const auto & point_indices : sideFacePartTriangleIndices3D(side_face_part, length_tol))
    triangles.push_back({side_face_part.points[point_indices[0]],
                         side_face_part.points[point_indices[1]],
                         side_face_part.points[point_indices[2]]});

  return triangles;
}

/* There are a lot of boolean-returning methods here: They are used to distinguish boundary
 * treatments, and all return differently depending on the geometry an input mesh may have.*/
static bool
samePoint3D(const Point & point0, const Point & point1, const Real tol = 1e-12)
{
  return MooseUtils::absoluteFuzzyEqual(point0, point1, tol);
}

static bool
sameSegment3D(const Point & point0,
              const Point & point1,
              const Point & other_point0,
              const Point & other_point1,
              const Real tol = 1e-12)
{
  return (samePoint3D(point0, other_point0, tol) && samePoint3D(point1, other_point1, tol)) ||
         (samePoint3D(point0, other_point1, tol) && samePoint3D(point1, other_point0, tol));
}

static bool
pointOnSegment3D(const Point & point,
                 const Point & segment_point0,
                 const Point & segment_point1,
                 const Real tol = 1e-12)
{
  const Point segment = segment_point1 - segment_point0;
  const Real segment_length = segment.norm();

  if (segment_length <= tol)
    return samePoint3D(segment_point0, point, tol);

  const Point point_delta = point - segment_point0;

  if (libMesh::cross_norm(segment, point_delta) > tol * segment_length)
    return false;

  const Real parameter = (point_delta * segment) / (segment_length * segment_length);
  const Real parameter_tol = tol / segment_length;

  return parameter >= -parameter_tol && parameter <= 1.0 + parameter_tol;
}

static void
addUniqueSplitEdgePoint3D(std::vector<SplitEdgePoint3D> & split_edge_points,
                          const SplitEdgePoint3D & split_edge_point,
                          const Real tol = 1e-12)
{
  for (const auto & existing_split_edge_point : split_edge_points)
    if (sameSegment3D(existing_split_edge_point.edge_point0,
                      existing_split_edge_point.edge_point1,
                      split_edge_point.edge_point0,
                      split_edge_point.edge_point1,
                      tol) &&
        samePoint3D(existing_split_edge_point.split_point, split_edge_point.split_point, tol))
      return;

  split_edge_points.push_back(split_edge_point);
}

static bool
samePointSet3D(const std::vector<Point> & points,
               const std::array<Point, 4> & other_points,
               const Real tol = 1e-12)
{
  if (points.size() != other_points.size())
    return false;

  for (const auto & point : points)
  {
    bool point_found = false;

    for (const auto & other_point : other_points)
      if (samePoint3D(point, other_point, tol))
      {
        point_found = true;
        break;
      }

    if (!point_found)
      return false;
  }

  return true;
}

// In Split and Polycut, it is clearly possible we could end up with two of the same face as
// promising candidates. It is necessary to not try this.
static bool
sameSplitFaceReplacement3D(const SplitFaceReplacement3D & face_replacement0,
                           const SplitFaceReplacement3D & face_replacement1,
                           const Real tol = 1e-12)
{
  std::vector<Point> replacement0_points;

  for (const auto & point : face_replacement0.original_face_points)
    replacement0_points.push_back(point);

  return samePointSet3D(replacement0_points, face_replacement1.original_face_points, tol);
}

// Adds the replacement faces
static void
addUniqueSplitFaceReplacement3D(std::vector<SplitFaceReplacement3D> & face_replacements,
                                const SplitFaceReplacement3D & face_replacement,
                                const Real tol = 1e-12)
{
  for (const auto & existing_face_replacement : face_replacements)
    if (sameSplitFaceReplacement3D(existing_face_replacement, face_replacement, tol))
      return;

  face_replacements.push_back(face_replacement);
}

// Reads child faces and makes sure their orientation matches the parent orientation
static std::vector<Point>
orientedSplitFacePart3D(const std::array<Point, 4> & face_part,
                        const Point & reference_normal,
                        const Real tol = 1e-12)
{
  std::vector<Point> oriented_face_part(face_part.begin(), face_part.end());
  const Point normal = faceNormal3D(oriented_face_part, tol);

  if (normal.norm() > tol && reference_normal.norm() > tol && normal * reference_normal < 0.0)
    std::reverse(oriented_face_part.begin(), oriented_face_part.end());

  return oriented_face_part;
}

using SplitRoundedPointKey3D = std::array<long long, 3>;
using SplitRoundedSegmentKey3D = std::pair<SplitRoundedPointKey3D, SplitRoundedPointKey3D>;

struct SplitPointIndex3D
{
  Real tol;
  Real key_tol;
  std::map<SplitRoundedPointKey3D, std::vector<std::size_t>> split_edge_points;
  std::map<SplitRoundedPointKey3D, std::vector<std::size_t>> face_replacements;
};

static SplitRoundedPointKey3D
splitRoundedPointKey3D(const Point & point, const Real key_tol)
{
  return SplitRoundedPointKey3D{{static_cast<long long>(std::llround(point(0) / key_tol)),
                                 static_cast<long long>(std::llround(point(1) / key_tol)),
                                 static_cast<long long>(std::llround(point(2) / key_tol))}};
}

static SplitRoundedSegmentKey3D
splitRoundedSegmentKey3D(const Point & point0, const Point & point1, const Real key_tol)
{
  auto key0 = splitRoundedPointKey3D(point0, key_tol);
  auto key1 = splitRoundedPointKey3D(point1, key_tol);

  if (key1 < key0)
    std::swap(key0, key1);

  return SplitRoundedSegmentKey3D{key0, key1};
}

static SplitPointIndex3D
buildSplitPointIndex3D(const std::vector<SplitEdgePoint3D> & split_edge_points,
                       const std::vector<SplitFaceReplacement3D> & face_replacements,
                       const Real tol = 1e-12)
{
  SplitPointIndex3D point_index{tol, std::max(tol, Real(1e-12)), {}, {}};
  std::map<SplitRoundedSegmentKey3D, std::vector<std::size_t>> split_edge_segments;

  // Makes sure split-edge and replacement-face (for split/polycut) candidate lists do not contain
  // duplicate record indices.
  const auto addCandidate = [](std::vector<std::size_t> & candidates, const std::size_t candidate)
  {
    for (const auto existing_candidate : candidates)
      if (existing_candidate == candidate)
        return;

    candidates.push_back(candidate);
  };

  for (const auto split_edge_point_index : index_range(split_edge_points))
  {
    const auto & split_edge_point = split_edge_points[split_edge_point_index];
    addCandidate(
        split_edge_segments[splitRoundedSegmentKey3D(
            split_edge_point.edge_point0, split_edge_point.edge_point1, point_index.key_tol)],
        split_edge_point_index);
  }

  for (const auto & split_edge_segment : split_edge_segments)
    for (const auto split_edge_point_index : split_edge_segment.second)
    {
      const auto & split_edge_point = split_edge_points[split_edge_point_index];
      addCandidate(point_index.split_edge_points[splitRoundedPointKey3D(
                       split_edge_point.edge_point0, point_index.key_tol)],
                   split_edge_point_index);
      addCandidate(point_index.split_edge_points[splitRoundedPointKey3D(
                       split_edge_point.edge_point1, point_index.key_tol)],
                   split_edge_point_index);

      for (const auto source_edge_split_point_index : split_edge_segment.second)
        addCandidate(
            point_index.split_edge_points[splitRoundedPointKey3D(
                split_edge_points[source_edge_split_point_index].split_point, point_index.key_tol)],
            split_edge_point_index);
    }

  for (const auto face_replacement_index : index_range(face_replacements))
    for (const auto & point : face_replacements[face_replacement_index].original_face_points)
      addCandidate(
          point_index.face_replacements[splitRoundedPointKey3D(point, point_index.key_tol)],
          face_replacement_index);

  return point_index;
}

// Split needs to add additional nodes along edges, and additional faces (child faces) in lieu of
// the faces we are splitting. These nodes are also added to their neighboring elems to maintain
// conformality
static void
insertSplitPointsOnSideEdges3D(std::vector<std::vector<Point>> & side_points,
                               const std::vector<SplitEdgePoint3D> & split_edge_points,
                               const std::vector<SplitFaceReplacement3D> & face_replacements,
                               const SplitPointIndex3D & split_point_index)
{
  if (split_edge_points.empty() && face_replacements.empty())
    return;

  const Real tol = split_point_index.tol;

  // Keeps first-seen candidate ordering while suppressing duplicate record indices.
  const auto addCandidate = [](std::vector<std::size_t> & candidates, const std::size_t candidate)
  {
    for (const auto existing_candidate : candidates)
      if (existing_candidate == candidate)
        return;

    candidates.push_back(candidate);
  };

  // Gathers split-edge candidates
  const auto appendSplitEdgePointCandidates =
      [&](const Point & point, std::vector<std::size_t> & candidates)
  {
    const auto point_key = splitRoundedPointKey3D(point, split_point_index.key_tol);

    for (const auto dx : {-1, 0, 1})
      for (const auto dy : {-1, 0, 1})
        for (const auto dz : {-1, 0, 1})
        {
          const SplitRoundedPointKey3D neighbor_key{
              {point_key[0] + dx, point_key[1] + dy, point_key[2] + dz}};
          const auto index_it = split_point_index.split_edge_points.find(neighbor_key);

          if (index_it == split_point_index.split_edge_points.end())
            continue;

          for (const auto split_edge_point_index : index_it->second)
            addCandidate(candidates, split_edge_point_index);
        }
  };

  const auto appendFaceReplacementCandidates =
      [&](const Point & point, std::vector<std::size_t> & candidates)
  {
    const auto point_key = splitRoundedPointKey3D(point, split_point_index.key_tol);

    for (const auto dx : {-1, 0, 1})
      for (const auto dy : {-1, 0, 1})
        for (const auto dz : {-1, 0, 1})
        {
          const SplitRoundedPointKey3D neighbor_key{
              {point_key[0] + dx, point_key[1] + dy, point_key[2] + dz}};
          const auto index_it = split_point_index.face_replacements.find(neighbor_key);

          if (index_it == split_point_index.face_replacements.end())
            continue;

          for (const auto face_replacement_index : index_it->second)
            addCandidate(candidates, face_replacement_index);
        }
  };

  std::vector<std::vector<Point>> split_side_points;
  split_side_points.reserve(side_points.size());

  for (const auto & side : side_points)
  {
    bool replaced_side = false;

    if (side.size() == 4 && !face_replacements.empty())
    {
      std::vector<std::size_t> candidate_face_replacement_indices;

      for (const auto & point : side)
        appendFaceReplacementCandidates(point, candidate_face_replacement_indices);

      for (const auto face_replacement_index : candidate_face_replacement_indices)
      {
        const auto & face_replacement = face_replacements[face_replacement_index];

        if (samePointSet3D(side, face_replacement.original_face_points, tol))
        {
          const Point side_normal = faceNormal3D(side, tol);

          addSidePoints3D(
              split_side_points,
              orientedSplitFacePart3D(face_replacement.child0_face_points, side_normal, tol),
              tol);
          addSidePoints3D(
              split_side_points,
              orientedSplitFacePart3D(face_replacement.child1_face_points, side_normal, tol),
              tol);
          replaced_side = true;
          break;
        }
      }
    }

    if (replaced_side)
      continue;

    std::vector<Point> split_side;
    std::vector<std::size_t> inserted_split_point_indices;
    split_side.reserve(side.size());

    // Adds a new side point
    const auto addOrderedPoint = [&](const Point & point)
    {
      if (!split_side.empty() && samePoint3D(split_side.back(), point, tol))
        return split_side.size() - 1;

      split_side.push_back(point);
      return split_side.size() - 1;
    };

    // Records inserted split-point indices
    const auto addInsertedSplitPointIndex = [&](const std::size_t point_index)
    {
      for (const auto existing_point_index : inserted_split_point_indices)
        if (existing_point_index == point_index)
          return;

      inserted_split_point_indices.push_back(point_index);
    };

    for (const auto i : index_range(side))
    {
      const Point & point0 = side[i];
      const Point & point1 = side[(i + 1) % side.size()];
      const Point edge = point1 - point0;
      const Real edge_length_sq = edge.norm_sq();

      addOrderedPoint(point0);

      if (edge_length_sq <= tol * tol)
        continue;

      std::vector<std::pair<Real, Point>> edge_split_points;
      std::vector<std::size_t> candidate_split_edge_point_indices;

      appendSplitEdgePointCandidates(point0, candidate_split_edge_point_indices);
      appendSplitEdgePointCandidates(point1, candidate_split_edge_point_indices);

      for (const auto split_edge_point_index : candidate_split_edge_point_indices)
      {
        const auto & split_edge_point = split_edge_points[split_edge_point_index];

        if (samePoint3D(split_edge_point.split_point, point0, tol) ||
            samePoint3D(split_edge_point.split_point, point1, tol) ||
            !pointOnSegment3D(split_edge_point.split_point, point0, point1, tol))
          continue;

        const Real parameter = ((split_edge_point.split_point - point0) * edge) / edge_length_sq;
        bool already_added = false;

        for (const auto & edge_split_point : edge_split_points)
          if (samePoint3D(edge_split_point.second, split_edge_point.split_point, tol))
          {
            already_added = true;
            break;
          }

        if (!already_added)
          edge_split_points.push_back({parameter, split_edge_point.split_point});
      }

      if (edge_split_points.size() > 1)
        std::sort(edge_split_points.begin(),
                  edge_split_points.end(),
                  [](const auto & a, const auto & b) { return a.first < b.first; });

      for (const auto & edge_split_point : edge_split_points)
      {
        const std::size_t point_index = addOrderedPoint(edge_split_point.second);
        addInsertedSplitPointIndex(point_index);
      }
    }

    if (split_side.size() > 1 && samePoint3D(split_side.front(), split_side.back(), tol))
      split_side.pop_back();

    // Splits polyhonal side across two inserted split points (SPLIT concave treatment)
    const auto addSplitSideParts =
        [&](const std::size_t point_index0, const std::size_t point_index1)
    {
      if (point_index0 == point_index1)
        return false;

      const std::size_t first_index = std::min(point_index0, point_index1);
      const std::size_t second_index = std::max(point_index0, point_index1);
      std::vector<Point> split_side0;
      std::vector<Point> split_side1;

      for (const auto point_index : make_range(first_index, second_index + 1))
        split_side0.push_back(split_side[point_index]);

      for (const auto point_index : make_range(second_index, split_side.size()))
        split_side1.push_back(split_side[point_index]);

      for (const auto point_index : make_range(std::size_t(0), first_index + 1))
        split_side1.push_back(split_side[point_index]);

      std::vector<std::vector<Point>> split_face_parts;
      addSidePoints3D(split_face_parts, split_side0, tol);
      addSidePoints3D(split_face_parts, split_side1, tol);

      if (split_face_parts.size() == 2)
      {
        split_side_points.insert(
            split_side_points.end(), split_face_parts.begin(), split_face_parts.end());
        return true;
      }

      return false;
    };

    bool split_side_was_split = false;

    if (inserted_split_point_indices.size() == 2)
    {
      if (addSplitSideParts(inserted_split_point_indices[0], inserted_split_point_indices[1]))
        split_side_was_split = true;
    }
    else if (inserted_split_point_indices.size() == 1 && split_side.size() > 4)
    {
      const std::size_t inserted_index = inserted_split_point_indices[0];

      for (const auto offset : make_range(std::size_t(2), split_side.size() - 1))
      {
        const std::size_t opposite_index = (inserted_index + offset) % split_side.size();

        if (addSplitSideParts(inserted_index, opposite_index))
        {
          split_side_was_split = true;
          break;
        }
      }
    }

    if (split_side_was_split)
      continue;

    addSidePoints3D(split_side_points, split_side, tol);
  }

  side_points.swap(split_side_points);
}

static bool
faceContainsPoint3D(const std::vector<Point> & side_points,
                    const Point & point,
                    const Real tol = 1e-12)
{
  for (const auto & side_point : side_points)
    if (samePoint3D(side_point, point, tol))
      return true;

  return false;
}

static bool
faceContainsEdge3D(const std::vector<Point> & side_points,
                   const Point & point0,
                   const Point & point1,
                   const Real tol = 1e-12)
{
  if (side_points.size() < 2)
    return false;

  for (const auto i : index_range(side_points))
    if (sameSegment3D(
            side_points[i], side_points[(i + 1) % side_points.size()], point0, point1, tol))
      return true;

  return false;
}

static bool
addUniqueSegment3D(std::vector<std::pair<Point, Point>> & segments,
                   const Point & point0,
                   const Point & point1,
                   const Real tol = 1e-12)
{
  if (samePoint3D(point0, point1, tol))
    return false;

  for (const auto & segment : segments)
    if (sameSegment3D(segment.first, segment.second, point0, point1, tol))
      return false;

  segments.push_back({point0, point1});
  return true;
}

// More complicated concave polyhedrons, i.e., those with more than one reentrant edge, aren't
// handled cleanly by the split/polycut logic, so we use surface plane tests in that case.
static bool
hasPointOutsidePolyhedronFacePlanes3D(const std::vector<std::vector<Point>> & side_points,
                                      const Real length_tol)
{
  const Real area_tol = length_tol * length_tol;

  for (const auto & side : side_points)
  {
    if (side.size() < 3)
      continue;

    const Point side_centroid = centroid3D(side);
    Point normal = faceNormal3D(side, area_tol);

    if (normal.norm() <= area_tol)
      continue;

    normal /= normal.norm();
    bool has_positive_point = false;
    bool has_negative_point = false;

    for (const auto & other_side : side_points)
      for (const auto & point : other_side)
      {
        if (faceContainsPoint3D(side, point, length_tol))
          continue;

        const Real signed_distance = normal * (point - side_centroid);

        if (signed_distance > length_tol)
          has_positive_point = true;
        else if (signed_distance < -length_tol)
          has_negative_point = true;

        if (has_positive_point && has_negative_point)
          return true;
      }
  }

  return false;
}

// Surface normal test for polyhedrons with only one reentrant edge
static bool
findConcavePolyhedronEdge3D(const std::vector<std::vector<Point>> & side_points,
                            const Real normal_dot_tol,
                            const Real length_tol,
                            PolyCutEdge3D & concave_edge,
                            std::size_t * concave_side0_index = nullptr,
                            std::size_t * concave_side1_index = nullptr)
{
  const Real area_tol = length_tol * length_tol;
  const Point polyhedron_centroid = polyhedronCentroid3D(side_points, length_tol);
  std::vector<Point> side_normals(side_points.size());
  std::vector<Point> side_centroids(side_points.size());

  for (const auto i : index_range(side_points))
  {
    side_centroids[i] = centroid3D(side_points[i]);
    Point normal = faceNormal3D(side_points[i], area_tol);

    if (normal.norm() <= area_tol)
      continue;

    if (normal * (side_centroids[i] - polyhedron_centroid) < 0.0)
      normal = -1.0 * normal;

    side_normals[i] = normal / normal.norm();
  }

  for (const auto i : index_range(side_points))
    for (const auto j : make_range(i + 1, side_points.size()))
    {
      if (side_normals[i].norm() <= length_tol || side_normals[j].norm() <= length_tol)
        continue;

      if (side_normals[i] * side_normals[j] <= normal_dot_tol)
        continue;

      const Point centroid_delta = side_centroids[j] - side_centroids[i];

      if (side_normals[i] * centroid_delta <= length_tol ||
          side_normals[j] * (-1.0 * centroid_delta) <= length_tol)
        continue;

      for (const auto point_index : index_range(side_points[i]))
      {
        const Point & point0 = side_points[i][point_index];
        const Point & point1 = side_points[i][(point_index + 1) % side_points[i].size()];

        if (faceContainsEdge3D(side_points[j], point0, point1, length_tol))
        {
          concave_edge = {point0, point1};

          if (concave_side0_index)
            *concave_side0_index = i;
          if (concave_side1_index)
            *concave_side1_index = j;

          return true;
        }
      }
    }

  return false;
}

static bool
polyCutPointAssociation3D(const std::vector<std::vector<Point>> & side_points,
                          const Point & point,
                          const PolyCutEdge3D & concave_edge,
                          const Real tol,
                          unsigned int & concave_point_index)
{
  bool shares_point0 = false;
  bool shares_point1 = false;

  for (const auto & side : side_points)
  {
    if (!faceContainsPoint3D(side, point, tol))
      continue;

    const bool has_point0 = faceContainsPoint3D(side, concave_edge.p0, tol);
    const bool has_point1 = faceContainsPoint3D(side, concave_edge.p1, tol);

    if (has_point0 && has_point1)
      return false;

    shares_point0 = shares_point0 || has_point0;
    shares_point1 = shares_point1 || has_point1;
  }

  if (shares_point0 == shares_point1)
    return false;

  concave_point_index = shares_point0 ? 0 : 1;
  return true;
}

static std::vector<std::vector<Point>>
polyCutFaceCandidates3D(const std::vector<std::vector<Point>> & side_points,
                        const PolyCutEdge3D & concave_edge,
                        const Real tol)
{
  std::vector<std::pair<Point, Point>> real_edges;
  std::vector<Point> unique_points;

  for (const auto & side : side_points)
    for (const auto & point : side)
      addUniquePoint(unique_points, point, tol);

  // Assigns IDs for split candidate sorting
  const auto pointIndex = [&](const Point & point)
  {
    for (const auto i : index_range(unique_points))
      if (samePoint3D(unique_points[i], point, tol))
        return i;

    return unique_points.size();
  };

  struct PolyCutFaceCandidate3D
  {
    std::vector<Point> cut_face;
    std::size_t low_id;
    std::size_t high_id;
  };

  std::vector<PolyCutFaceCandidate3D> cut_face_candidates;

  for (const auto & side : side_points)
    for (const auto i : index_range(side))
      addUniqueSegment3D(real_edges, side[i], side[(i + 1) % side.size()], tol);

  for (const auto & real_edge : real_edges)
  {
    if (sameSegment3D(real_edge.first, real_edge.second, concave_edge.p0, concave_edge.p1, tol) ||
        samePoint3D(real_edge.first, concave_edge.p0, tol) ||
        samePoint3D(real_edge.first, concave_edge.p1, tol) ||
        samePoint3D(real_edge.second, concave_edge.p0, tol) ||
        samePoint3D(real_edge.second, concave_edge.p1, tol))
      continue;

    unsigned int first_association = 2;
    unsigned int second_association = 2;

    if (!polyCutPointAssociation3D(
            side_points, real_edge.first, concave_edge, tol, first_association) ||
        !polyCutPointAssociation3D(
            side_points, real_edge.second, concave_edge, tol, second_association) ||
        first_association == second_association)
      continue;

    const Point & point0_side_point = first_association == 0 ? real_edge.first : real_edge.second;
    const Point & point1_side_point = first_association == 1 ? real_edge.first : real_edge.second;
    std::vector<Point> cut_face = {
        concave_edge.p0, concave_edge.p1, point1_side_point, point0_side_point};

    if (hasNonzeroArea3D(cut_face, tol))
    {
      const auto point0_id = pointIndex(real_edge.first);
      const auto point1_id = pointIndex(real_edge.second);

      cut_face_candidates.push_back(
          {cut_face, std::min(point0_id, point1_id), std::max(point0_id, point1_id)});
    }
  }

  std::sort(cut_face_candidates.begin(),
            cut_face_candidates.end(),
            [](const auto & a, const auto & b)
            {
              if (a.low_id != b.low_id)
                return a.low_id < b.low_id;
              return a.high_id < b.high_id;
            });

  std::vector<std::vector<Point>> cut_faces;
  cut_faces.reserve(cut_face_candidates.size());

  for (const auto & cut_face_candidate : cut_face_candidates)
    cut_faces.push_back(cut_face_candidate.cut_face);

  return cut_faces;
}

static std::vector<SplitCutFaceCandidate3D>
splitCutFaceCandidates3D(const std::vector<std::vector<Point>> & side_points,
                         const PolyCutEdge3D & concave_edge,
                         const Real tol)
{
  struct SplitEligibleEdge3D
  {
    Point point0_side_point;
    Point point1_side_point;
    std::size_t low_id;
    std::size_t high_id;
  };

  std::vector<std::pair<Point, Point>> real_edges;
  std::vector<Point> unique_points;

  for (const auto & side : side_points)
    for (const auto & point : side)
      addUniquePoint(unique_points, point, tol);

  const auto pointIndex = [&](const Point & point)
  {
    for (const auto i : index_range(unique_points))
      if (samePoint3D(unique_points[i], point, tol))
        return i;

    return unique_points.size();
  };

  for (const auto & side : side_points)
    for (const auto i : index_range(side))
      addUniqueSegment3D(real_edges, side[i], side[(i + 1) % side.size()], tol);

  std::vector<SplitEligibleEdge3D> eligible_edges;

  for (const auto & real_edge : real_edges)
  {
    if (sameSegment3D(real_edge.first, real_edge.second, concave_edge.p0, concave_edge.p1, tol) ||
        samePoint3D(real_edge.first, concave_edge.p0, tol) ||
        samePoint3D(real_edge.first, concave_edge.p1, tol) ||
        samePoint3D(real_edge.second, concave_edge.p0, tol) ||
        samePoint3D(real_edge.second, concave_edge.p1, tol))
      continue;

    unsigned int first_association = 2;
    unsigned int second_association = 2;

    if (!polyCutPointAssociation3D(
            side_points, real_edge.first, concave_edge, tol, first_association) ||
        !polyCutPointAssociation3D(
            side_points, real_edge.second, concave_edge, tol, second_association) ||
        first_association == second_association)
      continue;

    const Point & point0_side_point = first_association == 0 ? real_edge.first : real_edge.second;
    const Point & point1_side_point = first_association == 1 ? real_edge.first : real_edge.second;
    const auto point0_id = pointIndex(real_edge.first);
    const auto point1_id = pointIndex(real_edge.second);

    eligible_edges.push_back({point0_side_point,
                              point1_side_point,
                              std::min(point0_id, point1_id),
                              std::max(point0_id, point1_id)});
  }

  std::sort(eligible_edges.begin(),
            eligible_edges.end(),
            [](const auto & a, const auto & b)
            {
              if (a.low_id != b.low_id)
                return a.low_id < b.low_id;
              return a.high_id < b.high_id;
            });

  std::vector<SplitCutFaceCandidate3D> cut_faces;

  for (const auto i : index_range(eligible_edges))
    for (const auto j : make_range(i + 1, eligible_edges.size()))
    {
      const Point point0_midpoint =
          0.5 * (eligible_edges[i].point0_side_point + eligible_edges[j].point0_side_point);
      const Point point1_midpoint =
          0.5 * (eligible_edges[i].point1_side_point + eligible_edges[j].point1_side_point);
      std::vector<Point> cut_face = {
          concave_edge.p0, concave_edge.p1, point1_midpoint, point0_midpoint};

      if (hasNonzeroArea3D(cut_face, tol))
      {
        const SplitFaceReplacement3D face_replacement = {{{eligible_edges[i].point0_side_point,
                                                           eligible_edges[i].point1_side_point,
                                                           eligible_edges[j].point1_side_point,
                                                           eligible_edges[j].point0_side_point}},
                                                         {{eligible_edges[i].point0_side_point,
                                                           eligible_edges[i].point1_side_point,
                                                           point1_midpoint,
                                                           point0_midpoint}},
                                                         {{point0_midpoint,
                                                           point1_midpoint,
                                                           eligible_edges[j].point1_side_point,
                                                           eligible_edges[j].point0_side_point}}};

        cut_faces.push_back({cut_face,
                             {{eligible_edges[i].point0_side_point,
                               eligible_edges[j].point0_side_point,
                               point0_midpoint},
                              {eligible_edges[i].point1_side_point,
                               eligible_edges[j].point1_side_point,
                               point1_midpoint}},
                             {face_replacement},
                             {},
                             {}});
      }
    }

  return cut_faces;
}

// Add a point generated while clipping a face, avoiding consecutive duplicate vertices.
static void
addClippedPoint3D(std::vector<Point> & points, const Point & point, const Real tol = 1e-12)
{
  if (!points.empty() && samePoint3D(points.back(), point, tol))
    return;

  points.push_back(point);
}

static void
cleanClippedFace3D(std::vector<Point> & points, const Real tol = 1e-12)
{
  if (points.size() > 1 && samePoint3D(points.front(), points.back(), tol))
    points.pop_back();
}

// Draws a plane around a set of points, and cuts the polyhedron about that plane. Used for PolyCut.
static std::vector<Point>
clipFaceToHalfspace3D(const std::vector<Point> & side_points,
                      const Point & plane_point,
                      const Point & plane_normal,
                      const bool keep_positive_side,
                      const Real tol = 1e-12)
{
  std::vector<Point> clipped_points;

  if (side_points.empty())
    return clipped_points;

  // Computes distance to a clipping plane
  const auto signedDistance = [&](const Point & point)
  { return plane_normal * (point - plane_point); };

  const auto isInside = [&](const Real signed_distance)
  { return keep_positive_side ? signed_distance >= -tol : signed_distance <= tol; };

  for (const auto i : index_range(side_points))
  {
    const Point & point0 = side_points[i];
    const Point & point1 = side_points[(i + 1) % side_points.size()];
    const Real distance0 = signedDistance(point0);
    const Real distance1 = signedDistance(point1);
    const bool inside0 = isInside(distance0);
    const bool inside1 = isInside(distance1);

    if (inside0 != inside1)
    {
      const Real denominator = distance0 - distance1;

      if (std::abs(denominator) > tol)
      {
        const Real t = std::max(Real(0), std::min(Real(1), distance0 / denominator));
        addClippedPoint3D(clipped_points, point0 + t * (point1 - point0), tol);
      }
    }

    if (inside1)
      addClippedPoint3D(clipped_points, point1, tol);
  }

  cleanClippedFace3D(clipped_points, tol);
  return clipped_points;
}

static void
orientPolyhedronSidePointsOutward3D(std::vector<std::vector<Point>> & side_points,
                                    const Real tol = 1e-12)
{
  const Point polyhedron_centroid = polyhedronCentroid3D(side_points, tol);

  for (auto & side : side_points)
  {
    const Point normal = faceNormal3D(side, tol);

    if (normal.norm() <= tol)
      continue;

    if (normal * (centroid3D(side) - polyhedron_centroid) < 0.0)
      std::reverse(side.begin(), side.end());
  }
}

static bool
buildPolyCutChildSidePoints3D(const std::vector<std::vector<Point>> & side_points,
                              const std::vector<Point> & cut_face,
                              const bool keep_positive_side,
                              const Real tol,
                              std::vector<std::vector<Point>> & child_side_points)
{
  Point plane_normal = faceNormal3D(cut_face, tol);

  if (plane_normal.norm() <= tol)
    return false;

  plane_normal /= plane_normal.norm();

  for (const auto & side : side_points)
  {
    const auto clipped_side =
        clipFaceToHalfspace3D(side, cut_face.front(), plane_normal, keep_positive_side, tol);

    addSidePoints3D(child_side_points, clipped_side, tol);
  }

  std::vector<Point> oriented_cut_face = cut_face;
  const Point cut_face_normal = faceNormal3D(oriented_cut_face, tol);
  const Point desired_cut_normal = keep_positive_side ? -1.0 * plane_normal : plane_normal;

  if (cut_face_normal * desired_cut_normal < 0.0)
    std::reverse(oriented_cut_face.begin(), oriented_cut_face.end());

  addSidePoints3D(child_side_points, oriented_cut_face, tol);
  orientPolyhedronSidePointsOutward3D(child_side_points, tol);

  return child_side_points.size() >= 4;
}

static std::vector<PolyCutResult3D>
polyCutSidePointCandidates3D(const std::vector<std::vector<Point>> & side_points,
                             const Real normal_dot_tol,
                             const Real length_tol)
{
  std::vector<PolyCutResult3D> results;
  PolyCutEdge3D concave_edge;

  if (!findConcavePolyhedronEdge3D(side_points, normal_dot_tol, length_tol, concave_edge))
    return results;

  for (const auto & cut_face : polyCutFaceCandidates3D(side_points, concave_edge, length_tol))
  {
    std::vector<std::vector<Point>> child0_side_points;
    std::vector<std::vector<Point>> child1_side_points;

    if (buildPolyCutChildSidePoints3D(
            side_points, cut_face, true, length_tol, child0_side_points) &&
        buildPolyCutChildSidePoints3D(side_points, cut_face, false, length_tol, child1_side_points))
      results.push_back({child0_side_points, child1_side_points});
  }

  return results;
}

// Scales tolerances based on the relative size of the polyhedra
static Real
polyhedronScale3D(const std::vector<std::vector<Point>> & side_points)
{
  Real scale = 1.0;

  for (const auto & side : side_points)
    for (const auto i : index_range(side))
      scale = std::max(scale, (side[(i + 1) % side.size()] - side[i]).norm());

  return scale;
}

static bool
pointOnTriangle3D(
    const Point & point, const Point & a, const Point & b, const Point & c, const Real tol = 1e-12)
{
  const Point v0 = b - a;
  const Point v1 = c - a;
  const Point v2 = point - a;
  const Point normal = v0.cross(v1);
  const Real normal_norm = normal.norm();

  if (normal_norm <= tol * tol)
    return false;

  if (std::abs(v2 * normal) > tol * normal_norm)
    return false;

  const Real d00 = v0 * v0;
  const Real d01 = v0 * v1;
  const Real d11 = v1 * v1;
  const Real d20 = v2 * v0;
  const Real d21 = v2 * v1;
  const Real denom = d00 * d11 - d01 * d01;

  if (std::abs(denom) <= tol * tol * tol * tol)
    return false;

  const Real bary_v = (d11 * d20 - d01 * d21) / denom;
  const Real bary_w = (d00 * d21 - d01 * d20) / denom;
  const Real bary_u = 1.0 - bary_v - bary_w;
  const Real parameter_tol = 1e-8;

  return bary_u >= -parameter_tol && bary_v >= -parameter_tol && bary_w >= -parameter_tol;
}

// Used to determine if points lie within the primal boundary
static bool
pointInsideTriangulatedSurface3D(const Point & point,
                                 const std::vector<std::vector<Point>> & surface_triangles,
                                 const Real tol = 1e-12)
{
  Real solid_angle = 0.0;

  for (const auto & triangle : surface_triangles)
  {
    if (triangle.size() != 3)
      continue;

    if (pointOnTriangle3D(point, triangle[0], triangle[1], triangle[2], tol))
      return true;

    const Point a = triangle[0] - point;
    const Point b = triangle[1] - point;
    const Point c = triangle[2] - point;
    const Real a_norm = a.norm();
    const Real b_norm = b.norm();
    const Real c_norm = c.norm();

    if (a_norm <= tol || b_norm <= tol || c_norm <= tol)
      return true;

    solid_angle += geom_utils::solidAngle(point, triangle[0], triangle[1], triangle[2]);
  }

  return std::abs(solid_angle) > libMesh::pi;
}

// Returns 6 times the volume of a tetrahedron
static Real
tetVolume6(const Point & point0, const Point & point1, const Point & point2, const Point & point3)
{
  return libMesh::triple_product(point1 - point0, point2 - point0, point3 - point0);
}

template <typename ValidSegment>
static bool
triangulateCentroidRingFace3D(const std::vector<Point> & side,
                              std::vector<std::vector<Point>> & surface_triangles,
                              const ValidSegment & valid_segment,
                              const DualPointSources3D & dual_point_sources,
                              bool & matched,
                              const Real tol = 1e-12)
{
  matched = false;

  if (side.size() < 4)
    return false;

  std::vector<bool> is_face_side_point;
  is_face_side_point.reserve(side.size());

  for (const auto & point : side)
  {
    const auto * const point_sources = findDualPointSources3D(dual_point_sources, point);

    if (!point_sources)
      return false;

    const auto hasSource = [&](const DualPointSource3D source)
    { return point_sources->count(source) > 0; };
    const bool is_primal_vertex = hasSource(DualPointSource3D::primal_vertex);
    const bool is_body_centroid = hasSource(DualPointSource3D::body_centroid);
    const bool is_face_side = hasSource(DualPointSource3D::face_centroid) ||
                              hasSource(DualPointSource3D::face_diagonal_midpoint) ||
                              hasSource(DualPointSource3D::boundary_edge_midpoint);

    if (is_primal_vertex)
    {
      if (point_sources->size() > 1)
        matched = true;

      return false;
    }

    if (is_face_side == is_body_centroid)
    {
      matched = true;
      return false;
    }

    is_face_side_point.push_back(is_face_side);
  }

  std::size_t n_body_points = 0;
  bool has_consecutive_body_points = false;

  for (const auto i : index_range(side))
    if (!is_face_side_point[i])
    {
      ++n_body_points;

      if (!is_face_side_point[(i + side.size() - 1) % side.size()] ||
          !is_face_side_point[(i + 1) % side.size()])
        has_consecutive_body_points = true;
    }

  const std::size_t n_face_points = side.size() - n_body_points;

  if (n_body_points == 0 || (!has_consecutive_body_points && n_face_points < 2))
    return false;

  std::size_t boundary_midpoint_index = side.size();

  if (has_consecutive_body_points)
  {
    for (const auto i : index_range(side))
    {
      const auto * const point_sources = findDualPointSources3D(dual_point_sources, side[i]);

      if (point_sources && point_sources->count(DualPointSource3D::boundary_edge_midpoint) > 0)
      {
        if (boundary_midpoint_index != side.size())
          return false;

        boundary_midpoint_index = i;
      }
    }

    if (boundary_midpoint_index == side.size())
      return false;
  }

  matched = true;
  Real face_scale = 0.0;

  for (const auto i : index_range(side))
    for (const auto j : make_range(i + 1, side.size()))
      face_scale = std::max(face_scale, (side[j] - side[i]).norm());

  if (face_scale <= tol)
    return false;

  const Real area_tol = tol * face_scale;

  // Interior faces can connect neighboring body centroids directly, leaving body-centroid runs
  // on a preserved boundary-edge ring. Fan from its unique primal-edge midpoint so no diagonal
  // cuts across the adjacent preserved boundary surface.
  if (has_consecutive_body_points)
  {
    std::vector<std::vector<Point>> midpoint_fan;
    midpoint_fan.reserve(side.size() - 2);

    const auto segmentHasEmbeddedPoint = [&](const std::size_t endpoint)
    {
      for (const auto other_point : index_range(side))
      {
        if (other_point == boundary_midpoint_index || other_point == endpoint)
          continue;

        if (pointOnSegment3D(side[other_point], side[boundary_midpoint_index], side[endpoint], tol))
          return true;
      }

      return false;
    };

    for (const auto offset : make_range(std::size_t(1), side.size() - 1))
    {
      const auto point1 = (boundary_midpoint_index + offset) % side.size();
      const auto point2 = (boundary_midpoint_index + offset + 1) % side.size();
      const bool point0_point1_is_diagonal = offset > 1;
      const bool point2_point0_is_diagonal = offset + 1 < side.size() - 1;
      std::vector<Point> triangle = {side[boundary_midpoint_index], side[point1], side[point2]};

      if ((point0_point1_is_diagonal && segmentHasEmbeddedPoint(point1)) ||
          (point2_point0_is_diagonal && segmentHasEmbeddedPoint(point2)) ||
          !hasNonzeroArea3D(triangle, area_tol) ||
          (point0_point1_is_diagonal &&
           !valid_segment(side[boundary_midpoint_index], side[point1])) ||
          (point2_point0_is_diagonal &&
           !valid_segment(side[boundary_midpoint_index], side[point2])))
        return false;

      midpoint_fan.push_back(std::move(triangle));
    }

    surface_triangles.insert(surface_triangles.end(), midpoint_fan.begin(), midpoint_fan.end());
    return true;
  }

  std::vector<Point> face_points;
  std::vector<std::vector<Point>> side_triangles;
  face_points.reserve(n_face_points);
  side_triangles.reserve(side.size() - 2);

  for (const auto i : index_range(side))
    if (is_face_side_point[i])
      face_points.push_back(side[i]);

  // Cut off each body-centroid corner between its neighboring face-side dual points.
  for (const auto i : index_range(side))
  {
    if (is_face_side_point[i])
      continue;

    const auto previous_i = (i + side.size() - 1) % side.size();
    const auto next_i = (i + 1) % side.size();
    const Point chord = side[next_i] - side[previous_i];
    const Real chord_length2 = chord.norm_sq();
    std::vector<Point> chord_points;

    if (chord_length2 <= tol * tol)
      return false;

    addUniquePoint(chord_points, side[previous_i], tol);

    for (const auto & face_point : face_points)
      if (pointOnSegment3D(face_point, side[previous_i], side[next_i], tol))
        addUniquePoint(chord_points, face_point, tol);

    addUniquePoint(chord_points, side[next_i], tol);

    std::sort(chord_points.begin(),
              chord_points.end(),
              [&](const Point & point0, const Point & point1)
              {
                return ((point0 - side[previous_i]) * chord) / chord_length2 <
                       ((point1 - side[previous_i]) * chord) / chord_length2;
              });

    for (const auto chord_i : make_range(chord_points.size() - 1))
    {
      std::vector<Point> triangle = {chord_points[chord_i], side[i], chord_points[chord_i + 1]};

      if (!hasNonzeroArea3D(triangle, area_tol))
        return false;

      if (!valid_segment(chord_points[chord_i], chord_points[chord_i + 1]))
        return false;

      side_triangles.push_back(std::move(triangle));
    }
  }

  // The corner triangles leave an inner polygon containing only face-side dual points.
  if (face_points.size() > 2 && hasNonzeroArea3D(face_points, area_tol))
  {
    std::vector<std::size_t> candidate_anchors;
    candidate_anchors.reserve(face_points.size());

    for (const auto i : index_range(face_points))
    {
      const auto * const point_sources = findDualPointSources3D(dual_point_sources, face_points[i]);

      if (point_sources && point_sources->count(DualPointSource3D::boundary_edge_midpoint) > 0)
        candidate_anchors.push_back(i);
    }

    if (candidate_anchors.empty())
      for (const auto i : index_range(face_points))
        candidate_anchors.push_back(i);

    // Fan from a primal-edge midpoint when present so no inner diagonal cuts past that midpoint.
    std::sort(candidate_anchors.begin(),
              candidate_anchors.end(),
              [&](const std::size_t i, const std::size_t j)
              { return pointKey3D(face_points[i]) < pointKey3D(face_points[j]); });

    bool built_inner_fan = false;

    for (const auto anchor : candidate_anchors)
    {
      std::vector<std::vector<Point>> inner_triangles;
      inner_triangles.reserve(face_points.size() - 2);
      bool valid_inner_fan = true;

      for (const auto offset : make_range(std::size_t(1), face_points.size() - 1))
      {
        const auto point1 = (anchor + offset) % face_points.size();
        const auto point2 = (anchor + offset + 1) % face_points.size();

        const auto segmentHasEmbeddedPoint = [&](const std::size_t endpoint)
        {
          for (const auto other_point : index_range(face_points))
          {
            if (other_point == anchor || other_point == endpoint)
              continue;

            if (pointOnSegment3D(
                    face_points[other_point], face_points[anchor], face_points[endpoint], tol))
              return true;
          }

          return false;
        };

        std::vector<Point> triangle = {
            face_points[anchor], face_points[point1], face_points[point2]};

        if ((offset > 1 && segmentHasEmbeddedPoint(point1)) ||
            (offset + 1 < face_points.size() - 1 && segmentHasEmbeddedPoint(point2)))
        {
          valid_inner_fan = false;
          break;
        }

        if (!hasNonzeroArea3D(triangle, area_tol))
        {
          valid_inner_fan = false;
          break;
        }

        if ((offset > 1 && !valid_segment(face_points[anchor], face_points[point1])) ||
            (offset + 1 < face_points.size() - 1 &&
             !valid_segment(face_points[anchor], face_points[point2])))
        {
          valid_inner_fan = false;
          break;
        }

        inner_triangles.push_back(std::move(triangle));
      }

      if (!valid_inner_fan)
        continue;

      side_triangles.insert(side_triangles.end(), inner_triangles.begin(), inner_triangles.end());
      built_inner_fan = true;
      break;
    }

    if (!built_inner_fan)
      return false;
  }

  surface_triangles.insert(surface_triangles.end(), side_triangles.begin(), side_triangles.end());
  return true;
}

template <typename ValidSegment>
static bool
triangulateSurfaceFace3D(const std::vector<Point> & side,
                         std::vector<std::vector<Point>> & surface_triangles,
                         const ValidSegment & valid_segment,
                         const Real tol = 1e-12,
                         const DualPointSources3D * const dual_point_sources = nullptr)
{
  if (side.size() < 3)
    return true;

  if (side.size() == 3)
  {
    if (!hasNonzeroArea3D(side, tol))
      return false;

    surface_triangles.push_back(side);
    return true;
  }

  // Preserve the centroid-ring topology even for planar sides. A selected primal diagonal
  // midpoint can lie on a body-centroid chord, which generic ear clipping could otherwise skip.
  if (dual_point_sources)
  {
    bool matched_centroid_ring = false;

    if (triangulateCentroidRingFace3D(side,
                                      surface_triangles,
                                      valid_segment,
                                      *dual_point_sources,
                                      matched_centroid_ring,
                                      tol))
      return true;

    if (matched_centroid_ring)
      return false;
  }

  Real face_scale = 0.0;

  for (const auto i : index_range(side))
    for (const auto j : make_range(i + 1, side.size()))
      face_scale = std::max(face_scale, (side[j] - side[i]).norm());

  if (face_scale <= tol)
    return false;

  const Real area_tol = tol * face_scale;
  Point normal = faceNormal3D(side, area_tol);

  if (normal.norm() <= area_tol)
    return false;

  normal /= normal.norm();

  Point e1;

  for (const auto i : index_range(side))
  {
    e1 = side[(i + 1) % side.size()] - side[i];
    e1 -= (e1 * normal) * normal;

    if (e1.norm() > tol)
    {
      e1 /= e1.norm();
      break;
    }
  }

  if (e1.norm() <= tol)
    return false;

  Point e2 = normal.cross(e1);

  if (e2.norm() <= tol)
    return false;

  e2 /= e2.norm();

  const Point center = centroid3D(side);
  std::vector<Point> projected_points;
  projected_points.reserve(side.size());

  for (const auto & point : side)
  {
    const Point point_delta = point - center;
    projected_points.push_back(Point(point_delta * e1, point_delta * e2, 0.0));
  }

  const Real signed_area = polygonSignedArea2D(projected_points);

  if (std::abs(signed_area) <= area_tol)
    return false;

  const Real orientation = signed_area > 0.0 ? 1.0 : -1.0;
  std::vector<std::size_t> remaining_indices;
  remaining_indices.reserve(side.size());

  for (const auto i : index_range(side))
    remaining_indices.push_back(i);

  const auto pointInProjectedTriangle =
      [&](const Point & point, const Point & a, const Point & b, const Point & c)
  {
    const Real signed_area0 = cross2D(point, a, b);
    const Real signed_area1 = cross2D(point, b, c);
    const Real signed_area2 = cross2D(point, c, a);

    return (signed_area0 >= -area_tol && signed_area1 >= -area_tol && signed_area2 >= -area_tol) ||
           (signed_area0 <= area_tol && signed_area1 <= area_tol && signed_area2 <= area_tol);
  };

  const auto segmentHasEmbeddedSidePoint =
      [&](const std::size_t point0_index, const std::size_t point1_index)
  {
    for (const auto other_index : index_range(side))
    {
      if (other_index == point0_index || other_index == point1_index)
        continue;

      if (pointOnSegment3D(side[other_index], side[point0_index], side[point1_index], tol))
        return true;
    }

    return false;
  };

  std::vector<std::vector<Point>> side_triangles;
  side_triangles.reserve(side.size() - 2);

  while (remaining_indices.size() > 3)
  {
    std::vector<std::size_t> candidate_positions;
    candidate_positions.reserve(remaining_indices.size());

    for (const auto position : index_range(remaining_indices))
      candidate_positions.push_back(position);

    std::sort(candidate_positions.begin(),
              candidate_positions.end(),
              [&](const std::size_t position0, const std::size_t position1)
              {
                return pointKey3D(side[remaining_indices[position0]]) <
                       pointKey3D(side[remaining_indices[position1]]);
              });

    bool clipped_ear = false;

    for (const auto position : candidate_positions)
    {
      const std::size_t previous_position =
          (position + remaining_indices.size() - 1) % remaining_indices.size();
      const std::size_t next_position = (position + 1) % remaining_indices.size();
      const std::size_t previous_index = remaining_indices[previous_position];
      const std::size_t current_index = remaining_indices[position];
      const std::size_t next_index = remaining_indices[next_position];

      const Real corner_cross = cross2D(projected_points[previous_index],
                                        projected_points[current_index],
                                        projected_points[next_index]);

      if (orientation * corner_cross <= area_tol)
        continue;

      std::vector<Point> triangle = {side[previous_index], side[current_index], side[next_index]};

      if (!hasNonzeroArea3D(triangle, area_tol))
        continue;

      bool contains_other_point = false;

      for (const auto other_index : remaining_indices)
      {
        if (other_index == previous_index || other_index == current_index ||
            other_index == next_index)
          continue;

        if (pointInProjectedTriangle(projected_points[other_index],
                                     projected_points[previous_index],
                                     projected_points[current_index],
                                     projected_points[next_index]))
        {
          contains_other_point = true;
          break;
        }
      }

      if (contains_other_point)
        continue;

      if (segmentHasEmbeddedSidePoint(previous_index, next_index))
        continue;

      if (!valid_segment(side[previous_index], side[next_index]))
        continue;

      side_triangles.push_back(std::move(triangle));
      remaining_indices.erase(remaining_indices.begin() + position);
      clipped_ear = true;
      break;
    }

    if (!clipped_ear)
      return false;
  }

  std::vector<Point> triangle = {
      side[remaining_indices[0]], side[remaining_indices[1]], side[remaining_indices[2]]};

  if (!hasNonzeroArea3D(triangle, area_tol))
    return false;

  side_triangles.push_back(std::move(triangle));
  surface_triangles.insert(surface_triangles.end(), side_triangles.begin(), side_triangles.end());
  return true;
}

// Triangulation, used for checking inside/outsideness of points, polyhedron watertightness,
// and netgen tetrahedralization.
template <typename ValidSegment>
static bool
surfaceTriangles3D(const std::vector<std::vector<Point>> & side_points,
                   std::vector<std::vector<Point>> & surface_triangles,
                   const ValidSegment & valid_segment,
                   const Real tol = 1e-12,
                   const DualPointSources3D * const dual_point_sources = nullptr)
{
  surface_triangles.clear();

  for (const auto & side : side_points)
  {
    if (side.size() < 3)
      continue;

    if (!triangulateSurfaceFace3D(side, surface_triangles, valid_segment, tol, dual_point_sources))
      return false;
  }

  return !surface_triangles.empty();
}

static bool
orientSurfaceTriangles3D(std::vector<std::vector<Point>> & surface_triangles,
                         const Real tol = 1e-12)
{
  if (surface_triangles.empty())
    return false;

  std::vector<Point> unique_points;
  std::vector<std::array<std::size_t, 3>> triangle_point_ids;
  triangle_point_ids.reserve(surface_triangles.size());

  const auto pointIndex = [&](const Point & point)
  {
    for (const auto i : index_range(unique_points))
      if (samePoint3D(unique_points[i], point, tol))
        return i;

    unique_points.push_back(point);
    return unique_points.size() - 1;
  };

  for (const auto & triangle : surface_triangles)
  {
    if (triangle.size() != 3)
      return false;

    std::array<std::size_t, 3> point_ids;

    for (const auto i : index_range(point_ids))
      point_ids[i] = pointIndex(triangle[i]);

    if (point_ids[0] == point_ids[1] || point_ids[1] == point_ids[2] ||
        point_ids[2] == point_ids[0])
      return false;

    triangle_point_ids.push_back(point_ids);
  }

  const auto surfaceEdgeKey = [](const std::size_t point0_id, const std::size_t point1_id)
  { return std::make_pair(std::min(point0_id, point1_id), std::max(point0_id, point1_id)); };

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::size_t>> edge_to_triangles;

  for (const auto triangle_index : index_range(triangle_point_ids))
  {
    const auto & point_ids = triangle_point_ids[triangle_index];

    for (const auto i : make_range(std::size_t(3)))
      edge_to_triangles[surfaceEdgeKey(point_ids[i], point_ids[(i + 1) % point_ids.size()])]
          .push_back(triangle_index);
  }

  for (const auto & edge_triangles : edge_to_triangles)
    if (edge_triangles.second.size() != 2)
      return false;

  const auto edgeDirection =
      [&](const std::size_t triangle_index,
          const std::pair<std::size_t, std::size_t> & edge) -> std::pair<std::size_t, std::size_t>
  {
    const auto & point_ids = triangle_point_ids[triangle_index];

    for (const auto i : make_range(std::size_t(3)))
    {
      const std::size_t point0_id = point_ids[i];
      const std::size_t point1_id = point_ids[(i + 1) % point_ids.size()];

      if (surfaceEdgeKey(point0_id, point1_id) == edge)
        return {point0_id, point1_id};
    }

    return {unique_points.size(), unique_points.size()};
  };

  std::vector<bool> oriented_triangles(triangle_point_ids.size(), false);

  for (const auto start_triangle : index_range(triangle_point_ids))
  {
    if (oriented_triangles[start_triangle])
      continue;

    std::vector<std::size_t> triangle_stack = {start_triangle};
    oriented_triangles[start_triangle] = true;

    while (!triangle_stack.empty())
    {
      const std::size_t triangle_index = triangle_stack.back();
      triangle_stack.pop_back();
      const auto & point_ids = triangle_point_ids[triangle_index];

      for (const auto i : make_range(std::size_t(3)))
      {
        const auto edge = surfaceEdgeKey(point_ids[i], point_ids[(i + 1) % point_ids.size()]);
        const auto & adjacent_triangles = edge_to_triangles[edge];
        const std::size_t neighbor_triangle =
            adjacent_triangles[0] == triangle_index ? adjacent_triangles[1] : adjacent_triangles[0];
        const auto direction = edgeDirection(triangle_index, edge);
        const auto neighbor_direction = edgeDirection(neighbor_triangle, edge);

        if (direction.first == unique_points.size() ||
            neighbor_direction.first == unique_points.size())
          return false;

        if (!oriented_triangles[neighbor_triangle])
        {
          if (direction == neighbor_direction)
            std::swap(triangle_point_ids[neighbor_triangle][1],
                      triangle_point_ids[neighbor_triangle][2]);

          oriented_triangles[neighbor_triangle] = true;
          triangle_stack.push_back(neighbor_triangle);
        }
        else if (direction == neighbor_direction)
          return false;
      }
    }
  }

  for (const auto & edge_triangles : edge_to_triangles)
  {
    const auto direction0 = edgeDirection(edge_triangles.second[0], edge_triangles.first);
    const auto direction1 = edgeDirection(edge_triangles.second[1], edge_triangles.first);

    if (direction0.first == unique_points.size() || direction1.first == unique_points.size() ||
        direction0 == direction1)
      return false;
  }

  const Point reference_point = centroid3D(unique_points);
  Real volume6 = 0.0;

  for (const auto & point_ids : triangle_point_ids)
    volume6 += tetVolume6(reference_point,
                          unique_points[point_ids[0]],
                          unique_points[point_ids[1]],
                          unique_points[point_ids[2]]);

  if (std::abs(volume6) <= tol * tol * tol)
    return false;

  if (volume6 < 0.0)
    for (auto & point_ids : triangle_point_ids)
      std::swap(point_ids[1], point_ids[2]);

  for (const auto triangle_index : index_range(surface_triangles))
    for (const auto i : make_range(std::size_t(3)))
      surface_triangles[triangle_index][i] = unique_points[triangle_point_ids[triangle_index][i]];

  return true;
}

// NetGen expects a conforming TRI3 surface. Topological edge pairing is not enough for surfaces
// with embedded vertices or self-intersections, which can make NetGen fail below exception
// handling.
static bool
surfaceTrianglesAreConforming3D(const std::vector<std::vector<Point>> & surface_triangles,
                                const Real tol = 1e-12)
{
  const auto triangleContainsPoint = [&](const std::vector<Point> & triangle, const Point & point)
  {
    for (const auto & triangle_point : triangle)
      if (samePoint3D(triangle_point, point, tol))
        return true;

    return false;
  };

  const auto sharedPoints =
      [&](const std::vector<Point> & triangle0, const std::vector<Point> & triangle1)
  {
    std::vector<Point> shared_points;

    for (const auto & point0 : triangle0)
      for (const auto & point1 : triangle1)
        if (samePoint3D(point0, point1, tol))
          addUniquePoint(shared_points, point0, tol);

    return shared_points;
  };

  const auto intersectionIsOnSharedFeature =
      [&](const Point & intersection, const std::vector<Point> & shared_points)
  {
    for (const auto & shared_point : shared_points)
      if (samePoint3D(intersection, shared_point, tol))
        return true;

    if (shared_points.size() >= 2)
      return pointOnSegment3D(intersection, shared_points[0], shared_points[1], tol);

    return false;
  };

  const auto segmentIntersectsTriangle = [&](const Point & segment_point0,
                                             const Point & segment_point1,
                                             const std::vector<Point> & triangle,
                                             Point & intersection)
  {
    const Point normal = (triangle[1] - triangle[0]).cross(triangle[2] - triangle[0]);
    const Real normal_norm = normal.norm();

    if (normal_norm <= tol * tol)
      return false;

    // A non-coplanar segment can intersect the triangle's plane only once. When one endpoint is a
    // triangle vertex, that shared endpoint is the intersection; avoid perturbing it in the
    // line-plane solve below.
    if (triangleContainsPoint(triangle, segment_point0) ||
        triangleContainsPoint(triangle, segment_point1))
      return false;

    const Real signed_distance0 = normal * (segment_point0 - triangle[0]);
    const Real signed_distance1 = normal * (segment_point1 - triangle[0]);
    const Real plane_tol = tol * normal_norm;

    if (std::abs(signed_distance0) <= plane_tol && std::abs(signed_distance1) <= plane_tol)
      return false;

    if ((signed_distance0 > plane_tol && signed_distance1 > plane_tol) ||
        (signed_distance0 < -plane_tol && signed_distance1 < -plane_tol))
      return false;

    const Real denominator = signed_distance0 - signed_distance1;

    if (std::abs(denominator) <= plane_tol)
      return false;

    const Real parameter = signed_distance0 / denominator;
    const Real segment_length = (segment_point1 - segment_point0).norm();
    const Real parameter_tol = segment_length > tol ? tol / segment_length : tol;

    if (parameter < -parameter_tol || parameter > 1.0 + parameter_tol)
      return false;

    intersection = segment_point0 + parameter * (segment_point1 - segment_point0);
    return pointOnTriangle3D(intersection, triangle[0], triangle[1], triangle[2], tol);
  };

  const auto trianglesAreCoplanar =
      [&](const std::vector<Point> & triangle0, const std::vector<Point> & triangle1)
  {
    const Point normal0 = (triangle0[1] - triangle0[0]).cross(triangle0[2] - triangle0[0]);
    const Point normal1 = (triangle1[1] - triangle1[0]).cross(triangle1[2] - triangle1[0]);
    const Real normal0_norm = normal0.norm();
    const Real normal1_norm = normal1.norm();

    if (normal0_norm <= tol * tol || normal1_norm <= tol * tol)
      return false;

    Real scale = 0.0;

    for (const auto edge_i : make_range(std::size_t(3)))
    {
      scale =
          std::max(scale, (triangle0[(edge_i + 1) % triangle0.size()] - triangle0[edge_i]).norm());
      scale =
          std::max(scale, (triangle1[(edge_i + 1) % triangle1.size()] - triangle1[edge_i]).norm());
    }

    if (scale <= tol)
      return false;

    const Real angular_tol = std::min(Real(1.0), tol / scale);

    if (normal0.cross(normal1).norm() > angular_tol * normal0_norm * normal1_norm)
      return false;

    for (const auto & point : triangle1)
      if (std::abs(normal0 * (point - triangle0[0])) > tol * normal0_norm)
        return false;

    return true;
  };

  const auto coplanarEdgesCross = [&](const Point & point00,
                                      const Point & point01,
                                      const Point & point10,
                                      const Point & point11,
                                      const Point & plane_normal)
  {
    unsigned int dropped_component = 0;

    for (const auto component : make_range(std::size_t(1), Moose::dim))
      if (std::abs(plane_normal(component)) > std::abs(plane_normal(dropped_component)))
        dropped_component = component;

    const auto projectPoint = [&](const Point & point)
    {
      Point projected_point;
      unsigned int projected_component = 0;

      for (const auto component : make_range(Moose::dim))
        if (component != dropped_component)
          projected_point(projected_component++) = point(component);

      return projected_point;
    };

    const Point projected00 = projectPoint(point00);
    const Point projected01 = projectPoint(point01);
    const Point projected10 = projectPoint(point10);
    const Point projected11 = projectPoint(point11);
    const Real segment_scale =
        std::max((projected01 - projected00).norm(), (projected11 - projected10).norm());
    const Real area_tol = tol * std::max(segment_scale, tol);
    const Real cross000 = cross2D(projected00, projected01, projected10);
    const Real cross001 = cross2D(projected00, projected01, projected11);
    const Real cross100 = cross2D(projected10, projected11, projected00);
    const Real cross101 = cross2D(projected10, projected11, projected01);
    const auto strictlyStraddles = [&](const Real value0, const Real value1)
    {
      return (value0 > area_tol && value1 < -area_tol) || (value0 < -area_tol && value1 > area_tol);
    };

    return strictlyStraddles(cross000, cross001) && strictlyStraddles(cross100, cross101);
  };

  for (const auto & triangle : surface_triangles)
  {
    if (triangle.size() != 3 || !hasNonzeroArea3D(triangle, tol * tol))
      return false;

    for (const auto edge_i : make_range(std::size_t(3)))
    {
      const Point & edge_point0 = triangle[edge_i];
      const Point & edge_point1 = triangle[(edge_i + 1) % triangle.size()];

      for (const auto & other_triangle : surface_triangles)
      {
        for (const auto & point : other_triangle)
        {
          if (samePoint3D(point, edge_point0, tol) || samePoint3D(point, edge_point1, tol))
            continue;

          if (pointOnSegment3D(point, edge_point0, edge_point1, tol))
            return false;
        }
      }
    }
  }

  for (const auto triangle0_i : index_range(surface_triangles))
    for (const auto triangle1_i : make_range(triangle0_i + 1, surface_triangles.size()))
    {
      const auto & triangle0 = surface_triangles[triangle0_i];
      const auto & triangle1 = surface_triangles[triangle1_i];
      const auto shared_points = sharedPoints(triangle0, triangle1);
      const bool triangles_are_coplanar = trianglesAreCoplanar(triangle0, triangle1);

      for (const auto & point : triangle0)
      {
        if (!triangleContainsPoint(triangle1, point) &&
            pointOnTriangle3D(point, triangle1[0], triangle1[1], triangle1[2], tol))
          return false;
      }

      for (const auto & point : triangle1)
      {
        if (!triangleContainsPoint(triangle0, point) &&
            pointOnTriangle3D(point, triangle0[0], triangle0[1], triangle0[2], tol))
          return false;
      }

      for (const auto edge_i : make_range(std::size_t(3)))
      {
        Point intersection;

        if (segmentIntersectsTriangle(triangle0[edge_i],
                                      triangle0[(edge_i + 1) % triangle0.size()],
                                      triangle1,
                                      intersection) &&
            !intersectionIsOnSharedFeature(intersection, shared_points))
          return false;

        if (triangles_are_coplanar)
          for (const auto other_edge_i : make_range(std::size_t(3)))
          {
            const Point & edge0_point0 = triangle0[edge_i];
            const Point & edge0_point1 = triangle0[(edge_i + 1) % triangle0.size()];
            const Point & edge1_point0 = triangle1[other_edge_i];
            const Point & edge1_point1 = triangle1[(other_edge_i + 1) % triangle1.size()];

            if (sameSegment3D(edge0_point0, edge0_point1, edge1_point0, edge1_point1, tol))
              continue;

            const Point plane_normal =
                (triangle0[1] - triangle0[0]).cross(triangle0[2] - triangle0[0]);

            if (coplanarEdgesCross(
                    edge0_point0, edge0_point1, edge1_point0, edge1_point1, plane_normal))
              return false;
          }

        if (segmentIntersectsTriangle(triangle1[edge_i],
                                      triangle1[(edge_i + 1) % triangle1.size()],
                                      triangle0,
                                      intersection) &&
            !intersectionIsOnSharedFeature(intersection, shared_points))
          return false;
      }
    }

  return true;
}

static bool
c0PolyhedronSidePoints3D(const std::vector<std::vector<Point>> & side_points,
                         std::vector<std::vector<Point>> & c0_side_points,
                         const Real tol = 1e-12,
                         const DualPointSources3D * const dual_point_sources = nullptr)
{
  const auto validSegment = [](const Point &, const Point &) { return true; };

  return surfaceTriangles3D(side_points, c0_side_points, validSegment, tol, dual_point_sources);
}

static std::shared_ptr<libMesh::Polygon>
buildC0Polygon3D(std::vector<Node *> nodes)
{
  // Polyhedron face keys retain polygon node order. Give independently constructed triangular
  // neighbors the same key; larger polygons must retain their cyclic geometric order.
  if (nodes.size() == 3)
    std::sort(nodes.begin(),
              nodes.end(),
              [](const Node * const node0, const Node * const node1)
              { return node0->id() < node1->id(); });

  auto polygon = std::make_shared<libMesh::C0Polygon>(nodes.size());

  for (const auto i : index_range(nodes))
    polygon->set_node(i, nodes[i]);

  return polygon;
}

// Original surface fan selection used by the pre-NetGen treatments.
template <typename ValidSegment>
static bool
preNetgenSurfaceTriangles3D(const std::vector<std::vector<Point>> & side_points,
                            std::vector<std::vector<Point>> & surface_triangles,
                            const ValidSegment & valid_segment,
                            const Real tol = 1e-12)
{
  surface_triangles.clear();

  for (const auto & side : side_points)
  {
    if (side.size() < 3)
      continue;

    bool added_side_triangles = false;

    for (const auto anchor : index_range(side))
    {
      std::vector<std::vector<Point>> side_triangles;
      bool valid_fan = true;

      for (const auto i : make_range(std::size_t(1), side.size() - 1))
      {
        const Point & point0 = side[anchor];
        const Point & point1 = side[(anchor + i) % side.size()];
        const Point & point2 = side[(anchor + i + 1) % side.size()];
        const bool point0_point1_is_diagonal = i > 1;
        const bool point2_point0_is_diagonal = i < side.size() - 2;

        if ((point0_point1_is_diagonal && !valid_segment(point0, point1)) ||
            (point2_point0_is_diagonal && !valid_segment(point2, point0)))
        {
          valid_fan = false;
          break;
        }

        std::vector<Point> triangle = {point0, point1, point2};

        if (hasNonzeroArea3D(triangle, tol))
          side_triangles.push_back(std::move(triangle));
      }

      if (valid_fan)
      {
        surface_triangles.insert(
            surface_triangles.end(), side_triangles.begin(), side_triangles.end());
        added_side_triangles = true;
        break;
      }
    }

    if (!added_side_triangles)
      return false;
  }

  return !surface_triangles.empty();
}

// Split and polycut historically used this fan decomposition before C0Polyhedron construction.
static bool
preNetgenC0PolyhedronSidePoints3D(const std::vector<std::vector<Point>> & side_points,
                                  std::vector<std::vector<Point>> & c0_side_points,
                                  const Real tol = 1e-12)
{
  const auto valid_segment = [](const Point &, const Point &) { return true; };
  return preNetgenSurfaceTriangles3D(side_points, c0_side_points, valid_segment, tol);
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  auto input_mesh = std::move(_input);

  if (!input_mesh->is_prepared())
    input_mesh->find_neighbors();

  const bool use_voronoi = _dual_mesh_type == "voronoi";
  const unsigned int mesh_dimension = input_mesh->mesh_dimension();

  if (mesh_dimension != 2 && mesh_dimension != 3)
    paramError("input", "DualMeshGenerator currently only supports 2D and 3D Meshes");
  if (mesh_dimension == 3 && use_voronoi)
    paramError("dual_mesh_type", "DualMeshGenerator does not support Voronoi duals for 3D meshes");
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
  if (mesh_dimension == 3)
    return generate3D(std::move(input_mesh));

  return generate2D(std::move(input_mesh));
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate3D(std::unique_ptr<MeshBase> input_mesh)
{

  const bool use_split = _concave_treatment.contains("split");
  const bool use_netgen = _concave_treatment.contains("netgen");
  const bool buffer_dual_cells = use_split || use_netgen;
  auto dualMesh = buildReplicatedMesh(3);
  const auto preserve_primal_subdomain_ids = preservedPrimalSubdomainIDs(*input_mesh);
  const BoundaryInfo & input_boundary_info = input_mesh->get_boundary_info();
  BoundaryInfo & dual_boundary_info = dualMesh->get_boundary_info();

  dual_boundary_info.set_sideset_name_map() = input_boundary_info.get_sideset_name_map();
  dual_boundary_info.set_nodeset_name_map() = input_boundary_info.get_nodeset_name_map();

  // Subdomain & interface preservation helpers, 3D-specific
  using NodeSubdomainKey = std::pair<dof_id_type, SubdomainID>;
  using EdgeSubdomainKey = std::pair<std::pair<dof_id_type, dof_id_type>, SubdomainID>;
  const SubdomainID merged_subdomain_key = Elem::invalid_subdomain_id;
  const auto preservePrimalSubdomain = [&](const SubdomainID subdomain_id)
  { return preserve_primal_subdomain_ids.count(subdomain_id) > 0; };
  const auto dualizeElem = [&](const Elem & elem)
  { return !preservePrimalSubdomain(elem.subdomain_id()); };
  const auto subdomainKey = [&](const SubdomainID subdomain_id)
  { return _preserve_subdomain_interfaces ? subdomain_id : merged_subdomain_key; };
  const auto nodeSubdomainKey = [&](const dof_id_type node_id, const SubdomainID subdomain_id)
  { return NodeSubdomainKey{node_id, subdomainKey(subdomain_id)}; };
  const auto edgeSubdomainKey =
      [&](const dof_id_type node0, const dof_id_type node1, const SubdomainID subdomain_id)
  { return EdgeSubdomainKey{edgeKey(node0, node1), subdomainKey(subdomain_id)}; };
  const auto preservedSide = [&](const Elem & elem, const unsigned int side)
  {
    const auto * const neighbor = elem.neighbor_ptr(side);
    return neighbor == nullptr ||
           (neighbor != nullptr && preservePrimalSubdomain(neighbor->subdomain_id())) ||
           (_preserve_subdomain_interfaces && neighbor->subdomain_id() != elem.subdomain_id());
  };
  const auto input_bounding_box = MeshTools::create_bounding_box(*input_mesh);
  const Point mesh_extent = input_bounding_box.max() - input_bounding_box.min();
  const Real mesh_scale =
      std::max(std::max(std::abs(mesh_extent(0)), std::abs(mesh_extent(1))),
               std::max(std::abs(mesh_extent(2)), std::numeric_limits<Real>::min()));
  const Real primal_boundary_length_tol =
      std::max(_geometry_relative_tol, Real(1e-12)) * mesh_scale;

  std::map<NodeSubdomainKey, std::vector<BoundaryFaceNormal3D>> boundary_node_normals;
  std::map<EdgeSubdomainKey, std::vector<Point>> boundary_edge_normals;
  std::map<SubdomainID, std::vector<std::vector<Point>>> primal_boundary_surface_triangles;
  DualPointSources3D dual_point_sources;

  for (const auto & elem : input_mesh->element_ptr_range())
  {
    if (!dualizeElem(*elem))
      continue;

    const Point elem_centroid = elem->true_centroid();
    addDualPointSource3D(dual_point_sources, elem_centroid, DualPointSource3D::body_centroid);

    // Gatherin surface normals, boundary info, etc from the primal mesh
    for (const auto side : elem->side_index_range())
    {
      const bool side_is_preserved = preservedSide(*elem, side);

      auto side_elem = elem->build_side_ptr(side);
      std::vector<dof_id_type> side_node_ids;
      std::vector<Point> side_points;
      side_node_ids.reserve(side_elem->n_vertices());
      side_points.reserve(side_elem->n_vertices());

      for (const auto n : make_range(side_elem->n_vertices()))
      {
        side_node_ids.push_back(side_elem->node_id(n));
        side_points.push_back(side_elem->point(n));
      }

      std::pair<dof_id_type, dof_id_type> primal_split_diagonal;
      const bool has_primal_split_diagonal =
          quad4PrimalSplitDiagonal3D(side_node_ids, primal_split_diagonal);

      for (const auto & side_face_part :
           sideFaceParts3D(side_node_ids,
                           side_points,
                           primal_boundary_length_tol,
                           has_primal_split_diagonal ? &primal_split_diagonal : nullptr))
      {
        const auto & side_face_points = side_face_part.points;
        const Point face_centroid = sideFacePartDualPoint3D(side_face_part);

        if (!side_is_preserved)
          continue;

        addDualPointSource3D(dual_point_sources,
                             face_centroid,
                             side_face_part.has_selected_diagonal
                                 ? DualPointSource3D::face_diagonal_midpoint
                                 : DualPointSource3D::face_centroid);

        // Record the two boundary planes without treating their shared diagonal as a primal edge.
        if (side_face_points.size() == 4 && side_face_part.has_selected_diagonal)
        {
          for (const auto & point_indices :
               sideFacePartTriangleIndices3D(side_face_part, primal_boundary_length_tol))
          {
            Point normal =
                (side_face_points[point_indices[1]] - side_face_points[point_indices[0]])
                    .cross(side_face_points[point_indices[2]] - side_face_points[point_indices[0]]);

            if (normal.norm() <= 1e-12)
              continue;

            if (normal * (elem_centroid - face_centroid) > 0.0)
              normal = -1.0 * normal;

            // NetGen chord repairs also need the primal surface for inside/outside checks, even
            // when surface-diagonal preservation is not requested.
            if (_preserve_diagonals || use_netgen)
            {
              std::vector<Point> triangle = {side_face_points[point_indices[0]],
                                             side_face_points[point_indices[1]],
                                             side_face_points[point_indices[2]]};
              const Point triangle_normal =
                  (triangle[1] - triangle[0]).cross(triangle[2] - triangle[0]);

              if (triangle_normal * normal < 0.0)
                std::swap(triangle[1], triangle[2]);

              primal_boundary_surface_triangles[subdomainKey(elem->subdomain_id())].push_back(
                  std::move(triangle));
            }

            for (const auto point_index : point_indices)
              addUniqueBoundaryFaceNormal3D(
                  boundary_node_normals[nodeSubdomainKey(side_face_part.node_ids[point_index],
                                                         elem->subdomain_id())],
                  normal,
                  face_centroid);

            for (const auto point_index : index_range(point_indices))
            {
              const dof_id_type node0 = side_face_part.node_ids[point_indices[point_index]];
              const dof_id_type node1 =
                  side_face_part.node_ids[point_indices[(point_index + 1) % point_indices.size()]];

              if (edgeKey(node0, node1) == side_face_part.selected_diagonal)
                continue;

              addUniqueDirection3D(
                  boundary_edge_normals[edgeSubdomainKey(node0, node1, elem->subdomain_id())],
                  normal);
            }
          }

          continue;
        }

        Point normal = faceNormal3D(side_face_points);

        if (normal.norm() <= 1e-12)
          continue;

        // Make sure face normals are oriented properly
        if (normal * (elem_centroid - face_centroid) > 0.0)
          normal = -1.0 * normal;

        if (_preserve_diagonals || use_netgen)
        {
          auto & boundary_surface_triangles =
              primal_boundary_surface_triangles[subdomainKey(elem->subdomain_id())];

          for (auto triangle : sideFacePartTriangles3D(side_face_part, primal_boundary_length_tol))
          {
            const Point triangle_normal =
                (triangle[1] - triangle[0]).cross(triangle[2] - triangle[0]);

            if (triangle_normal.norm() <= primal_boundary_length_tol * primal_boundary_length_tol)
              continue;

            if (triangle_normal * normal < 0.0)
              std::swap(triangle[1], triangle[2]);

            boundary_surface_triangles.push_back(triangle);
          }
        }

        for (const auto n : index_range(side_face_part.node_ids))
          addUniqueBoundaryFaceNormal3D(boundary_node_normals[nodeSubdomainKey(
                                            side_face_part.node_ids[n], elem->subdomain_id())],
                                        normal,
                                        face_centroid);

        for (const auto n : index_range(side_face_part.node_ids))
        {
          const dof_id_type node0 = side_face_part.node_ids[n];
          const dof_id_type node1 =
              side_face_part.node_ids[(n + 1) % side_face_part.node_ids.size()];

          addUniqueDirection3D(
              boundary_edge_normals[edgeSubdomainKey(node0, node1, elem->subdomain_id())], normal);
        }
      }
    }
  }

  std::set<NodeSubdomainKey> boundary_vertex_nodes;

  for (const auto & node_normals : boundary_node_normals)
    if (node_normals.second.size() > 1)
      boundary_vertex_nodes.insert(node_normals.first);

  std::map<EdgeSubdomainKey, Point> boundary_edge_midpoints;
  std::map<std::pair<dof_id_type, dof_id_type>, Point> boundary_edge_midpoints_by_edge;

  // Looks at each boundary edge, and determines if it defines a "corner" edge through adjacent face
  // normals. If so, computes a midpoint, and stores it for later use
  for (const auto & edge_normals : boundary_edge_normals)
  {
    const auto & edge = edge_normals.first.first;
    const auto boundary_subdomain_key = edge_normals.first.second;

    if (edge_normals.second.size() > 1 &&
        boundary_vertex_nodes.count({edge.first, boundary_subdomain_key}) &&
        boundary_vertex_nodes.count({edge.second, boundary_subdomain_key}))
    {
      const Point midpoint =
          0.5 * (*input_mesh->node_ptr(edge.first) + *input_mesh->node_ptr(edge.second));
      boundary_edge_midpoints[edge_normals.first] = midpoint;
      boundary_edge_midpoints_by_edge[edge] = midpoint;
      addDualPointSource3D(dual_point_sources, midpoint, DualPointSource3D::boundary_edge_midpoint);
    }
  }

  const Real boundary_normal_dot_tol =
      std::cos(libMesh::pi - std::min(_boundary_node_angular_tol, libMesh::pi));

  // For determining concave boundary elems
  const auto hasConcaveBoundaryNormals = [&](const NodeSubdomainKey & node_subdomain_key)
  {
    const auto normals_it = boundary_node_normals.find(node_subdomain_key);

    if (normals_it == boundary_node_normals.end())
      return false;

    const auto & normals = normals_it->second;

    for (const auto i : index_range(normals))
      for (const auto j : make_range(i + 1, normals.size()))
        if (normals[i].normal * normals[j].normal > boundary_normal_dot_tol)
        {
          const Point centroid_delta = normals[j].face_centroid - normals[i].face_centroid;

          // Reentrant corners have outward normals pointing toward the opposite boundary face.
          if (normals[i].normal * centroid_delta > primal_boundary_length_tol &&
              normals[j].normal * (-1.0 * centroid_delta) > primal_boundary_length_tol)
            return true;
        }

    return false;
  };

  const auto boundaryEdgeMidpoint =
      [&](const EdgeSubdomainKey & edge_subdomain_key) -> const Point *
  {
    const auto midpoint_it = boundary_edge_midpoints.find(edge_subdomain_key);

    if (midpoint_it != boundary_edge_midpoints.end())
      return &midpoint_it->second;

    const auto shared_midpoint_it = boundary_edge_midpoints_by_edge.find(edge_subdomain_key.first);

    if (shared_midpoint_it != boundary_edge_midpoints_by_edge.end())
      return &shared_midpoint_it->second;

    return nullptr;
  };

  using RoundedPointKey3D = std::array<long long, 3>;
  std::map<RoundedPointKey3D, std::vector<Node *>> dual_nodes_by_key;
  const Real dual_node_tol = primal_boundary_length_tol;

  // Using this to look up nodes easily
  const auto roundedPointKey = [&](const Point & point)
  {
    const Real key_tol = std::max(dual_node_tol, Real(1e-12));
    return RoundedPointKey3D{{static_cast<long long>(std::llround(point(0) / key_tol)),
                              static_cast<long long>(std::llround(point(1) / key_tol)),
                              static_cast<long long>(std::llround(point(2) / key_tol))}};
  };

  struct PrimalNodeBoundaryIDs3D
  {
    Point point;
    std::vector<boundary_id_type> boundary_ids;
  };

  std::map<RoundedPointKey3D, std::vector<PrimalNodeBoundaryIDs3D>> primal_node_boundary_ids;

  for (const auto * const node : input_mesh->node_ptr_range())
  {
    std::vector<boundary_id_type> boundary_ids;
    input_boundary_info.boundary_ids(node, boundary_ids);

    if (!boundary_ids.empty())
      primal_node_boundary_ids[roundedPointKey(*node)].push_back({*node, boundary_ids});
  }

  const auto addPrimalNodeBoundaryIDs = [&](Node * const dual_node, const Point & point)
  {
    const auto boundary_ids_it = primal_node_boundary_ids.find(roundedPointKey(point));

    if (boundary_ids_it == primal_node_boundary_ids.end())
      return;

    for (const auto & candidate : boundary_ids_it->second)
      if (samePoint3D(candidate.point, point, dual_node_tol))
        dual_boundary_info.add_node(dual_node, candidate.boundary_ids);
  };

  const auto getDualNode = [&](const Point & point)
  {
    auto & candidate_nodes = dual_nodes_by_key[roundedPointKey(point)];

    for (auto * const node : candidate_nodes)
      if (MooseUtils::absoluteFuzzyEqual(*node, point, dual_node_tol))
        return std::make_pair(node, false);

    Node * const node = dualMesh->add_point(point);
    candidate_nodes.push_back(node);
    addPrimalNodeBoundaryIDs(node, point);
    return std::make_pair(node, true);
  };

  // If we want to preserve subdomains (not dualize them) we copy them over
  const auto copyPreservedPrimalElements = [&]()
  {
    if (preserve_primal_subdomain_ids.empty())
      return;

    std::map<dof_id_type, Node *> copied_nodes;

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
        dual_boundary_info.add_side(target_elem, target_side, ids_to_copy);
    };

    const auto getPreservedPrimalNode = [&](const dof_id_type source_node_id, const Point & point)
    {
      const auto copied_node_it = copied_nodes.find(source_node_id);

      if (copied_node_it != copied_nodes.end())
        return copied_node_it->second;

      Node * const node = getDualNode(point).first;
      copied_nodes[source_node_id] = node;
      return node;
    };

    for (const auto & elem : input_mesh->element_ptr_range())
    {
      if (dualizeElem(*elem))
        continue;

      auto primal_elem = elem->build(elem->type());

      for (const auto n : elem->node_index_range())
        primal_elem->set_node(n, getPreservedPrimalNode(elem->node_id(n), elem->point(n)));

      primal_elem->subdomain_id() = elem->subdomain_id();
      Elem * const added_elem = dualMesh->add_elem(std::move(primal_elem));

      for (const auto side : elem->side_index_range())
        copySideBoundaryIDs(elem, side, added_elem, {cast_int<unsigned short>(side)});
    }
  };

  copyPreservedPrimalElements();

  std::map<std::pair<SubdomainID, PointKey3D>, bool> primal_boundary_point_cache;
  std::map<std::pair<SubdomainID, SegmentKey3D>, bool> primal_boundary_segment_cache;

  struct PrimalEdgeSide3D
  {
    // Retains the primal-edge identity of a dual side until all neighboring cells are buffered.
    EdgeSubdomainKey edge_key;
    std::size_t side_index;
    Point midpoint;
    bool replaced = false;
  };

  struct SplitDualCellSidePoints3D
  {
    std::vector<std::vector<Point>> polycut_side_points;
    dof_id_type source_node_id;
    SubdomainID output_subdomain_id = Elem::invalid_subdomain_id;
    bool searched_concave_edge = false;
    bool has_concave_edge = false;
    PolyCutEdge3D concave_edge;
    bool force_tetrahedralize = false;
    bool has_split_plan = false;
    SplitCutFaceCandidate3D split_plan;
    bool has_boundary_chord_split_plan = false;
    SplitCutFaceCandidate3D boundary_chord_split_plan;
    std::vector<PrimalEdgeSide3D> primal_edge_sides;
    std::vector<const Elem *> primal_elems;
  };

  std::vector<SplitDualCellSidePoints3D> split_dual_cell_side_points;

  const auto pointInsidePrimalBoundary =
      [&](const SubdomainID boundary_subdomain_id, const Point & point)
  {
    const auto point_key = std::make_pair(boundary_subdomain_id, pointKey3D(point));
    const auto cached_point_it = primal_boundary_point_cache.find(point_key);

    if (cached_point_it != primal_boundary_point_cache.end())
      return cached_point_it->second;

    const auto surface_triangles_it = primal_boundary_surface_triangles.find(boundary_subdomain_id);

    if (surface_triangles_it == primal_boundary_surface_triangles.end())
    {
      primal_boundary_point_cache.emplace(point_key, false);
      return false;
    }

    const bool inside = pointInsideTriangulatedSurface3D(
        point, surface_triangles_it->second, primal_boundary_length_tol);

    primal_boundary_point_cache.emplace(point_key, inside);
    return inside;
  };

  // Samples a line segment between two points to see if that segment is inside the primal boundary.
  const auto segmentInsidePrimalBoundary =
      [&](const SubdomainID boundary_subdomain_id, const Point & point0, const Point & point1)
  {
    const auto segment_key = std::make_pair(boundary_subdomain_id, segmentKey3D(point0, point1));
    const auto cached_segment_it = primal_boundary_segment_cache.find(segment_key);

    if (cached_segment_it != primal_boundary_segment_cache.end())
      return cached_segment_it->second;

    for (const auto fraction : {0.25, 0.5, 0.75})
      if (!pointInsidePrimalBoundary(boundary_subdomain_id, point0 + fraction * (point1 - point0)))
      {
        primal_boundary_segment_cache.emplace(segment_key, false);
        return false;
      }

    primal_boundary_segment_cache.emplace(segment_key, true);
    return true;
  };

  std::map<NodeSubdomainKey, std::vector<const Elem *>> source_node_to_elems;

  for (const auto & elem : input_mesh->element_ptr_range())
  {
    // Skip preserved subdomains
    if (!dualizeElem(*elem))
      continue;

    for (const auto n : make_range(elem->n_vertices()))
    {
      source_node_to_elems[nodeSubdomainKey(elem->node_id(n), elem->subdomain_id())].push_back(
          elem); // Grabs the primal vertices we need to preserve geometry
      addDualPointSource3D(dual_point_sources, elem->point(n), DualPointSource3D::primal_vertex);
    }
  }

  struct PrimalBoundarySide3D
  {
    std::vector<std::vector<Point>> triangles;
    std::vector<boundary_id_type> boundary_ids;
  };

  struct PrimalBoundaryInfo3D
  {
    std::vector<PrimalBoundarySide3D> sides;
    std::vector<boundary_id_type> source_node_boundary_ids;
  };

  // Match boundary IDs after the final decomposition so direct, split, polycut, and NetGen
  // elements all use the same preservation path.
  const auto primalBoundaryInfo =
      [&](const std::vector<const Elem *> & primal_elems, const dof_id_type source_node_id)
  {
    PrimalBoundaryInfo3D boundary_info;
    input_boundary_info.boundary_ids(input_mesh->node_ptr(source_node_id),
                                     boundary_info.source_node_boundary_ids);
    std::sort(boundary_info.source_node_boundary_ids.begin(),
              boundary_info.source_node_boundary_ids.end());

    for (const auto * const elem : primal_elems)
      for (const auto side : elem->side_index_range())
      {
        std::vector<boundary_id_type> boundary_ids;
        input_boundary_info.boundary_ids(elem, side, boundary_ids);

        if (boundary_ids.empty())
          continue;

        auto side_elem = elem->build_side_ptr(side);
        std::vector<dof_id_type> side_node_ids;
        std::vector<Point> side_points;
        side_node_ids.reserve(side_elem->n_vertices());
        side_points.reserve(side_elem->n_vertices());

        for (const auto n : make_range(side_elem->n_vertices()))
        {
          side_node_ids.push_back(side_elem->node_id(n));
          side_points.push_back(side_elem->point(n));
        }

        std::pair<dof_id_type, dof_id_type> primal_split_diagonal;
        const bool has_primal_split_diagonal =
            quad4PrimalSplitDiagonal3D(side_node_ids, primal_split_diagonal);
        PrimalBoundarySide3D boundary_side{{}, boundary_ids};

        for (const auto & side_face_part :
             sideFaceParts3D(side_node_ids,
                             side_points,
                             primal_boundary_length_tol,
                             has_primal_split_diagonal ? &primal_split_diagonal : nullptr))
        {
          const auto side_triangles =
              sideFacePartTriangles3D(side_face_part, primal_boundary_length_tol);
          boundary_side.triangles.insert(
              boundary_side.triangles.end(), side_triangles.begin(), side_triangles.end());
        }

        if (!boundary_side.triangles.empty())
          boundary_info.sides.push_back(std::move(boundary_side));
      }

    return boundary_info;
  };

  const auto pointOnPrimalBoundarySide =
      [&](const Point & point, const PrimalBoundarySide3D & boundary_side)
  {
    for (const auto & triangle : boundary_side.triangles)
      if (pointOnTriangle3D(
              point, triangle[0], triangle[1], triangle[2], primal_boundary_length_tol))
        return true;

    return false;
  };

  const auto addPrimalSideBoundaryIDs =
      [&](Elem * const dual_elem, const PrimalBoundaryInfo3D & primal_boundary_info)
  {
    std::set<boundary_id_type> candidate_boundary_ids;

    for (const auto & boundary_side : primal_boundary_info.sides)
      candidate_boundary_ids.insert(boundary_side.boundary_ids.begin(),
                                    boundary_side.boundary_ids.end());

    for (const auto side : dual_elem->side_index_range())
    {
      auto side_elem = dual_elem->build_side_ptr(side);
      std::vector<Point> side_vertices;
      std::vector<Point> samples;
      side_vertices.reserve(side_elem->n_vertices());
      samples.reserve(2 * side_elem->n_vertices() + 1);

      for (const auto n : make_range(side_elem->n_vertices()))
      {
        side_vertices.push_back(side_elem->point(n));
        samples.push_back(side_elem->point(n));
        samples.push_back(
            0.5 * (side_elem->point(n) + side_elem->point((n + 1) % side_elem->n_vertices())));
      }

      samples.push_back(centroid3D(side_vertices));
      std::set<boundary_id_type> matched_boundary_ids;

      for (const auto boundary_id : candidate_boundary_ids)
      {
        bool all_samples_on_side = true;

        for (const auto & sample : samples)
        {
          bool sample_on_side = false;

          for (const auto & boundary_side : primal_boundary_info.sides)
            if (std::find(boundary_side.boundary_ids.begin(),
                          boundary_side.boundary_ids.end(),
                          boundary_id) != boundary_side.boundary_ids.end() &&
                pointOnPrimalBoundarySide(sample, boundary_side))
            {
              sample_on_side = true;
              break;
            }

          if (!sample_on_side)
          {
            all_samples_on_side = false;
            break;
          }
        }

        if (all_samples_on_side)
          matched_boundary_ids.insert(boundary_id);
      }

      if (!matched_boundary_ids.empty())
      {
        const std::vector<boundary_id_type> boundary_ids(matched_boundary_ids.begin(),
                                                         matched_boundary_ids.end());
        dual_boundary_info.add_side(dual_elem, cast_int<unsigned short>(side), boundary_ids);

        std::vector<boundary_id_type> nodeset_ids;

        std::set_intersection(boundary_ids.begin(),
                              boundary_ids.end(),
                              primal_boundary_info.source_node_boundary_ids.begin(),
                              primal_boundary_info.source_node_boundary_ids.end(),
                              std::back_inserter(nodeset_ids));

        if (!nodeset_ids.empty())
          for (const auto n : dual_elem->nodes_on_side(side))
            dual_boundary_info.add_node(dual_elem->node_ptr(n), nodeset_ids);
      }
    }
  };

  const auto polyhedronSurfaceCanBePassedToC0 =
      [&](const std::vector<std::vector<Point>> & c0_side_points, const Real length_tol)
  {
    std::vector<Point> unique_points;
    const Real area_tol = length_tol * length_tol;

    for (const auto & side : c0_side_points)
    {
      if (side.size() != 3 || !hasNonzeroArea3D(side, area_tol))
        return false;

      for (const auto & point : side)
        addUniquePoint(unique_points, point, length_tol);
    }

    if (unique_points.size() < 4)
      return false;

    const auto pointIndex = [&](const Point & point)
    {
      for (const auto i : index_range(unique_points))
        if (samePoint3D(unique_points[i], point, length_tol))
          return i;

      return unique_points.size();
    };

    std::map<std::pair<std::size_t, std::size_t>, unsigned int> edge_counts;
    std::map<std::pair<std::size_t, std::size_t>, unsigned int> directed_edge_counts;

    for (const auto & side : c0_side_points)
    {
      std::array<std::size_t, 3> point_ids;

      for (const auto i : make_range(std::size_t(3)))
      {
        point_ids[i] = pointIndex(side[i]);

        if (point_ids[i] == unique_points.size())
          return false;
      }

      if (point_ids[0] == point_ids[1] || point_ids[1] == point_ids[2] ||
          point_ids[2] == point_ids[0])
        return false;

      for (const auto i : make_range(std::size_t(3)))
      {
        const auto point0_id = point_ids[i];
        const auto point1_id = point_ids[(i + 1) % 3];

        edge_counts[{std::min(point0_id, point1_id), std::max(point0_id, point1_id)}]++;
        directed_edge_counts[{point0_id, point1_id}]++;
      }
    }

    if (edge_counts.empty())
      return false;

    for (const auto & edge_count : edge_counts)
    {
      if (edge_count.second != 2)
        return false;

      const auto point0_id = edge_count.first.first;
      const auto point1_id = edge_count.first.second;

      if (directed_edge_counts[{point0_id, point1_id}] != 1 ||
          directed_edge_counts[{point1_id, point0_id}] != 1)
        return false;
    }

    Real volume6 = 0.0;
    const Point & reference_point = unique_points.front();

    for (const auto & side : c0_side_points)
      volume6 += tetVolume6(reference_point, side[0], side[1], side[2]);

    if (std::abs(volume6) <= length_tol * length_tol * length_tol)
      return false;

    return !hasPointOutsidePolyhedronFacePlanes3D(c0_side_points, length_tol);
  };

  const auto canBuildPolyhedron = [&](MeshBase & mesh,
                                      const std::vector<std::vector<Point>> & side_points,
                                      const bool use_dual_point_sources = true,
                                      const bool require_convex_surface = true,
                                      const bool use_pre_netgen_triangulation = false) -> bool
  {
    if (side_points.size() < 4)
      return false;

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    std::vector<std::vector<Point>> c0_side_points;

    if (!(use_pre_netgen_triangulation
              ? preNetgenC0PolyhedronSidePoints3D(side_points, c0_side_points, length_tol)
              : c0PolyhedronSidePoints3D(side_points,
                                         c0_side_points,
                                         length_tol,
                                         use_dual_point_sources ? &dual_point_sources : nullptr)))
      return false;

    if (require_convex_surface && !polyhedronSurfaceCanBePassedToC0(c0_side_points, length_tol))
      return false;

    std::vector<Node *> local_nodes;
    std::vector<std::shared_ptr<libMesh::Polygon>> sides;

    const auto getLocalNode = [&](const Point & point)
    {
      for (auto * const node : local_nodes)
        if (MooseUtils::absoluteFuzzyEqual(*node, point))
          return node;

      Node * const node = mesh.add_point(point);
      local_nodes.push_back(node);

      return node;
    };

    sides.reserve(c0_side_points.size());

    for (const auto & side : c0_side_points)
    {
      std::vector<Node *> polygon_nodes;
      polygon_nodes.reserve(side.size());

      for (const auto & point : side)
        polygon_nodes.push_back(getLocalNode(point));

      sides.push_back(buildC0Polygon3D(std::move(polygon_nodes)));
    }

    std::unique_ptr<libMesh::Node> mid_elem_node;
    std::unique_ptr<libMesh::C0Polyhedron> dual_elem;

    libmesh_try { dual_elem = std::make_unique<libMesh::C0Polyhedron>(sides, mid_elem_node); }
    libmesh_catch(const libMesh::NotImplemented &) { return false; }
    libmesh_catch(const libMesh::LogicError &) { return false; }

    if (mid_elem_node)
      mesh.add_node(std::move(mid_elem_node));

    mesh.add_elem(std::move(dual_elem));

    return true;
  };

  const auto addPolyhedron = [&](const std::vector<std::vector<Point>> & side_points,
                                 const SubdomainID output_subdomain_id,
                                 const PrimalBoundaryInfo3D & boundary_info,
                                 const bool use_dual_point_sources = true,
                                 const bool use_pre_netgen_triangulation = false) -> bool
  {
    if (side_points.size() < 4)
      return false;

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    std::vector<std::vector<Point>> c0_side_points;
    std::vector<std::shared_ptr<libMesh::Polygon>> sides;

    if (!(use_pre_netgen_triangulation
              ? preNetgenC0PolyhedronSidePoints3D(side_points, c0_side_points, length_tol)
              : c0PolyhedronSidePoints3D(side_points,
                                         c0_side_points,
                                         length_tol,
                                         use_dual_point_sources ? &dual_point_sources : nullptr)))
      return false;

    const auto getOrCreateDualNode = [&](const Point & point) { return getDualNode(point).first; };

    sides.reserve(c0_side_points.size());

    for (const auto & side : c0_side_points)
    {
      std::vector<Node *> polygon_nodes;
      polygon_nodes.reserve(side.size());

      for (const auto & point : side)
        polygon_nodes.push_back(getOrCreateDualNode(point));

      sides.push_back(buildC0Polygon3D(std::move(polygon_nodes)));
    }

    std::unique_ptr<libMesh::Node> mid_elem_node;
    std::unique_ptr<libMesh::C0Polyhedron> dual_elem;

    libmesh_try { dual_elem = std::make_unique<libMesh::C0Polyhedron>(sides, mid_elem_node); }
    libmesh_catch(const libMesh::NotImplemented &) { return false; }
    libmesh_catch(const libMesh::LogicError &) { return false; }

    if (mid_elem_node)
      dualMesh->add_node(std::move(mid_elem_node));

    if (output_subdomain_id != Elem::invalid_subdomain_id)
      dual_elem->subdomain_id() = output_subdomain_id;
    Elem * const added_elem = dualMesh->add_elem(std::move(dual_elem));
    addPrimalSideBoundaryIDs(added_elem, boundary_info);

    return true;
  };

  // Polyhedra that are bad enough need to be triangulated and tetrahedralized via NetGen
  const auto addTetrahedralizedPolyhedron =
      [&](const std::vector<std::vector<Point>> & side_points,
          const SubdomainID output_subdomain_id,
          const bool touches_preserved_primal_boundary,
          const PrimalBoundaryInfo3D & boundary_info) -> std::size_t
  {
    if (side_points.size() < 4)
      return 0;

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    const Real volume_tol = length_tol * length_tol * length_tol;
    std::vector<std::vector<Point>> surface_triangles;

    // Rejects surface diagonals (on nonplanar quads) that violate the primal boundary geometry
    const auto validSurfaceSegment = [&](const Point & point0, const Point & point1)
    {
      return !_preserve_diagonals || !touches_preserved_primal_boundary ||
             segmentInsidePrimalBoundary(output_subdomain_id, point0, point1);
    };

    if (!surfaceTriangles3D(
            side_points, surface_triangles, validSurfaceSegment, length_tol, &dual_point_sources))
      return 0;

    if (!orientSurfaceTriangles3D(surface_triangles, length_tol))
      return 0;

    if (!surfaceTrianglesAreConforming3D(surface_triangles, length_tol))
      return 0;

    const auto addNetgenTetrahedralizedSurface = [&]()
    {
#ifdef LIBMESH_HAVE_NETGEN
      // NetGen is sensitive to very thin local coordinate scales. Tetrahedralize an
      // affine-normalized copy of the surface, then map generated tetrahedra back to the original
      // coordinates.
      Point min_point = surface_triangles.front().front();
      Point max_point = min_point;

      for (const auto & triangle : surface_triangles)
        for (const auto & point : triangle)
          for (const auto component : make_range(Moose::dim))
          {
            min_point(component) = std::min(min_point(component), point(component));
            max_point(component) = std::max(max_point(component), point(component));
          }

      const Point normalization_center = 0.5 * (min_point + max_point);
      const Point normalization_extent = max_point - min_point;
      const Real max_extent = std::max(std::max(normalization_extent(0), normalization_extent(1)),
                                       std::max(normalization_extent(2), length_tol));
      Point normalization_scale;

      for (const auto component : make_range(Moose::dim))
        normalization_scale(component) = normalization_extent(component) > length_tol
                                             ? 1.0 / normalization_extent(component)
                                             : 1.0 / max_extent;

      const auto normalizePoint = [&](const Point & point)
      {
        Point normalized_point;

        for (const auto component : make_range(Moose::dim))
          normalized_point(component) =
              (point(component) - normalization_center(component)) * normalization_scale(component);

        return normalized_point;
      };

      const auto denormalizePoint = [&](const Point & point)
      {
        Point denormalized_point;

        for (const auto component : make_range(Moose::dim))
          denormalized_point(component) =
              point(component) / normalization_scale(component) + normalization_center(component);

        return denormalized_point;
      };

      std::vector<std::vector<Point>> netgen_surface_triangles = surface_triangles;

      for (auto & triangle : netgen_surface_triangles)
        for (auto & point : triangle)
          point = normalizePoint(point);

      const Real netgen_polyhedron_scale = polyhedronScale3D(netgen_surface_triangles);
      const Real netgen_length_tol = tol * netgen_polyhedron_scale;
      const Real netgen_volume_tol = netgen_length_tol * netgen_length_tol * netgen_length_tol;
      const Real netgen_desired_volume =
          std::max(netgen_volume_tol,
                   netgen_polyhedron_scale * netgen_polyhedron_scale * netgen_polyhedron_scale);
      auto netgen_mesh = buildReplicatedMesh(3);
      std::vector<Node *> boundary_nodes;

      const auto getBoundaryNode = [&](const Point & point)
      {
        for (auto * const node : boundary_nodes)
          if (MooseUtils::absoluteFuzzyEqual(*node, point, netgen_length_tol))
            return node;

        Node * const node = netgen_mesh->add_point(point);
        boundary_nodes.push_back(node);

        return node;
      };

      for (const auto & triangle : netgen_surface_triangles)
      {
        auto tri = std::make_unique<Tri3>();

        for (const auto n : make_range(std::size_t(3)))
          tri->set_node(n, getBoundaryNode(triangle[n]));

        netgen_mesh->add_elem(std::move(tri));
      }

      netgen_mesh->prepare_for_use();

      libMesh::NetGenMeshInterface netgen(*netgen_mesh);
      netgen.smooth_after_generating() = false;
      netgen.desired_volume() = netgen_desired_volume;

#ifdef LIBMESH_ENABLE_EXCEPTIONS
      try
      {
        netgen.triangulate();
      }
      catch (...)
      {
        return std::size_t(0);
      }
#else
      netgen.triangulate();
#endif

      std::vector<std::array<Point, 4>> generated_tets;

      for (const auto & elem : netgen_mesh->element_ptr_range())
      {
        if (elem->dim() != 3 || elem->n_vertices() != 4)
          continue;

        std::array<Point, 4> tet_points = {denormalizePoint(elem->point(0)),
                                           denormalizePoint(elem->point(1)),
                                           denormalizePoint(elem->point(2)),
                                           denormalizePoint(elem->point(3))};

        if (std::abs(tetVolume6(tet_points[0], tet_points[1], tet_points[2], tet_points[3])) <=
            volume_tol)
          continue;

        generated_tets.push_back(tet_points);
      }

      if (generated_tets.empty())
        return std::size_t(0);

      for (const auto & tet_points : generated_tets)
      {
        auto tet = std::make_unique<Tet4>();
        tet->set_node(0, getDualNode(tet_points[0]).first);
        tet->set_node(1, getDualNode(tet_points[1]).first);

        if (tetVolume6(tet_points[0], tet_points[1], tet_points[2], tet_points[3]) > 0.0)
        {
          tet->set_node(2, getDualNode(tet_points[2]).first);
          tet->set_node(3, getDualNode(tet_points[3]).first);
        }
        else
        {
          tet->set_node(2, getDualNode(tet_points[3]).first);
          tet->set_node(3, getDualNode(tet_points[2]).first);
        }

        // Keep NetGen's tetrahedral decomposition, but store each child as a four-sided
        // C0Polyhedron so it retains its source subdomain with the other dual polyhedra.
        std::vector<std::shared_ptr<libMesh::Polygon>> tet_sides;
        tet_sides.reserve(tet->n_sides());

        for (const auto side_i : tet->side_index_range())
        {
          std::vector<Node *> side_nodes;
          side_nodes.reserve(Tet4::nodes_per_side);

          for (const auto node_i : make_range(Tet4::nodes_per_side))
            side_nodes.push_back(tet->node_ptr(Tet4::side_nodes_map[side_i][node_i]));

          tet_sides.push_back(buildC0Polygon3D(std::move(side_nodes)));
        }

        std::unique_ptr<libMesh::Node> mid_elem_node;
        auto tet_polyhedron = std::make_unique<libMesh::C0Polyhedron>(tet_sides, mid_elem_node);

        if (mid_elem_node)
          dualMesh->add_node(std::move(mid_elem_node));

        if (output_subdomain_id != Elem::invalid_subdomain_id)
          tet_polyhedron->subdomain_id() = output_subdomain_id;
        Elem * const added_polyhedron = dualMesh->add_elem(std::move(tet_polyhedron));
        addPrimalSideBoundaryIDs(added_polyhedron, boundary_info);
      }

      return generated_tets.size();
#else
      return std::size_t(0);
#endif
    };

    return addNetgenTetrahedralizedSurface();
  };

  const auto addPolyCutPolyhedra = [&](const std::vector<std::vector<Point>> & side_points,
                                       const SubdomainID output_subdomain_id,
                                       const PrimalBoundaryInfo3D & boundary_info) -> std::size_t
  {
    // Pre-NetGen treatment triangulates cut children without the direct/NetGen source topology.
    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    const auto polycut_candidates =
        polyCutSidePointCandidates3D(side_points, boundary_normal_dot_tol, length_tol);

    for (const auto & polycut_candidate : polycut_candidates)
    {
      auto validation_mesh = buildReplicatedMesh(3);

      if (!canBuildPolyhedron(
              *validation_mesh, polycut_candidate.child0_side_points, false, true, true) ||
          !canBuildPolyhedron(
              *validation_mesh, polycut_candidate.child1_side_points, false, true, true))
        continue;

      if (!addPolyhedron(polycut_candidate.child0_side_points,
                         output_subdomain_id,
                         boundary_info,
                         false,
                         true) ||
          !addPolyhedron(polycut_candidate.child1_side_points,
                         output_subdomain_id,
                         boundary_info,
                         false,
                         true))
        mooseError("Could not add polycut 3D dual polyhedron children.");

      return 2;
    }

    return 0;
  };

  const auto addSplitPolyhedra = [&](const std::vector<std::vector<Point>> & side_points,
                                     const SubdomainID output_subdomain_id,
                                     const PrimalBoundaryInfo3D & boundary_info,
                                     const SplitCutFaceCandidate3D * const split_plan =
                                         nullptr) -> std::size_t
  {
    if (split_plan && !split_plan->child0_side_points.empty() &&
        !split_plan->child1_side_points.empty())
    {
      if (!addPolyhedron(
              split_plan->child0_side_points, output_subdomain_id, boundary_info, false, true) ||
          !addPolyhedron(
              split_plan->child1_side_points, output_subdomain_id, boundary_info, false, true))
        mooseError("Could not add split 3D dual polyhedron children.");

      return 2;
    }

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;

    const auto addSplitCandidatePolyhedra =
        [&](const SplitCutFaceCandidate3D & split_candidate) -> bool
    {
      std::vector<std::vector<Point>> child0_side_points;
      std::vector<std::vector<Point>> child1_side_points;

      if (!buildPolyCutChildSidePoints3D(
              side_points, split_candidate.cut_face, true, length_tol, child0_side_points) ||
          !buildPolyCutChildSidePoints3D(
              side_points, split_candidate.cut_face, false, length_tol, child1_side_points))
        return false;

      auto validation_mesh = buildReplicatedMesh(3);

      if (!canBuildPolyhedron(*validation_mesh, child0_side_points, false, true, true) ||
          !canBuildPolyhedron(*validation_mesh, child1_side_points, false, true, true))
        return false;

      if (!addPolyhedron(child0_side_points, output_subdomain_id, boundary_info, false, true) ||
          !addPolyhedron(child1_side_points, output_subdomain_id, boundary_info, false, true))
        mooseError("Could not add split 3D dual polyhedron children.");

      return true;
    };

    if (split_plan)
      return addSplitCandidatePolyhedra(*split_plan) ? 2 : 0;

    PolyCutEdge3D concave_edge;

    if (!findConcavePolyhedronEdge3D(
            side_points, boundary_normal_dot_tol, length_tol, concave_edge))
      return 0;

    for (const auto & split_candidate :
         splitCutFaceCandidates3D(side_points, concave_edge, length_tol))
      if (addSplitCandidatePolyhedra(split_candidate))
        return 2;

    return 0;
  };

  const auto findValidSplitPlan = [&](const std::vector<std::vector<Point>> & side_points,
                                      const PolyCutEdge3D & concave_edge,
                                      SplitCutFaceCandidate3D & split_plan)
  {
    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;

    for (const auto & split_candidate :
         splitCutFaceCandidates3D(side_points, concave_edge, length_tol))
    {
      std::vector<std::vector<Point>> child0_side_points;
      std::vector<std::vector<Point>> child1_side_points;

      if (!buildPolyCutChildSidePoints3D(
              side_points, split_candidate.cut_face, true, length_tol, child0_side_points) ||
          !buildPolyCutChildSidePoints3D(
              side_points, split_candidate.cut_face, false, length_tol, child1_side_points))
        continue;

      auto validation_mesh = buildReplicatedMesh(3);

      if (!canBuildPolyhedron(*validation_mesh, child0_side_points, false, true, true) ||
          !canBuildPolyhedron(*validation_mesh, child1_side_points, false, true, true))
        continue;

      split_plan = split_candidate;
      split_plan.child0_side_points = std::move(child0_side_points);
      split_plan.child1_side_points = std::move(child1_side_points);
      return true;
    }

    return false;
  };

  // Determines what concave_treatment based on results
  const auto addPolyhedronOrTetrahedralize =
      [&](const std::vector<std::vector<Point>> & polycut_side_points,
          const bool force_tetrahedralize,
          const bool searched_concave_edge,
          const bool has_concave_edge,
          const dof_id_type source_node_id,
          const SubdomainID output_subdomain_id,
          const std::vector<const Elem *> & primal_elems,
          const SplitCutFaceCandidate3D * const split_plan = nullptr)
  {
    const bool concave_edge_search_failed =
        searched_concave_edge && !has_concave_edge && !split_plan;
    const auto boundary_info = primalBoundaryInfo(primal_elems, source_node_id);

    // First try the candidate directly before applying a configured concave treatment.
    if (!force_tetrahedralize && !has_concave_edge && !split_plan)
    {
      if (addPolyhedron(polycut_side_points, output_subdomain_id, boundary_info))
        return;
    }

    for (const auto & concave_treatment : _concave_treatment)
      if (concave_treatment == "split")
      {
        if (concave_edge_search_failed)
          continue;

        const std::size_t created_elements =
            addSplitPolyhedra(polycut_side_points, output_subdomain_id, boundary_info, split_plan);

        if (created_elements)
          return;
      }
      else if (concave_treatment == "polycut")
      {
        if (concave_edge_search_failed)
          continue;

        const std::size_t created_elements =
            addPolyCutPolyhedra(polycut_side_points, output_subdomain_id, boundary_info);

        if (created_elements)
          return;
      }
      else if (concave_treatment == "netgen")
      {
        bool touches_preserved_primal_boundary = false;

        if (_preserve_diagonals)
        {
          for (const auto * const elem : primal_elems)
          {
            for (const auto side : elem->side_index_range())
              if (preservedSide(*elem, side))
              {
                touches_preserved_primal_boundary = true;
                break;
              }

            if (touches_preserved_primal_boundary)
              break;
          }
        }

        const std::size_t netgen_elements =
            addTetrahedralizedPolyhedron(polycut_side_points,
                                         output_subdomain_id,
                                         touches_preserved_primal_boundary,
                                         boundary_info);

        if (netgen_elements)
          return;
      }

    mooseError("Could not resolve rejected non-convex 3D dual polyhedron.");
  };

  // Build one dual cell around each primal node. Concave cells are handled according to the
  // selected concave treatment.
  for (const auto & node_elems : source_node_to_elems)
  {
    const NodeSubdomainKey source_node_subdomain_key = node_elems.first;
    const dof_id_type source_node_id = source_node_subdomain_key.first;
    const SubdomainID source_subdomain_key = source_node_subdomain_key.second;
    const Point & source_point = *input_mesh->node_ptr(source_node_id);
    std::map<std::pair<dof_id_type, dof_id_type>, ConnectedFacePoints3D> edge_to_points;

    struct BoundaryPlaneEdgePoints3D
    {
      std::pair<dof_id_type, dof_id_type> edge;
      std::vector<Point> face_centroids;
      bool has_midpoint = false;
      Point midpoint;
    };

    struct BoundaryPlaneFacePoints3D
    {
      Point normal;
      Real plane_constant;
      ConnectedFacePoints3D face_points;
      std::vector<BoundaryPlaneEdgePoints3D> edge_points;
    };

    std::vector<BoundaryPlaneFacePoints3D> boundary_plane_faces;

    const auto pointOnBoundaryPlane =
        [&](const BoundaryPlaneFacePoints3D & boundary_plane_face, const Point & point)
    {
      return std::abs(boundary_plane_face.normal * point - boundary_plane_face.plane_constant) <=
             primal_boundary_length_tol;
    };

    const auto addBoundaryPlaneEdgePoints = [&](BoundaryPlaneFacePoints3D & boundary_plane_face,
                                                const dof_id_type other_node_id,
                                                const Point & face_centroid,
                                                const Point * midpoint)
    {
      const auto edge = edgeKey(source_node_id, other_node_id);

      std::size_t edge_index = boundary_plane_face.edge_points.size();

      for (const auto i : index_range(boundary_plane_face.edge_points))
        if (boundary_plane_face.edge_points[i].edge == edge)
        {
          edge_index = i;
          break;
        }

      if (edge_index == boundary_plane_face.edge_points.size())
        boundary_plane_face.edge_points.push_back({edge, {}, false, Point()});

      auto & boundary_edge_points = boundary_plane_face.edge_points[edge_index];

      addUniquePoint(
          boundary_edge_points.face_centroids, face_centroid, primal_boundary_length_tol);

      if (midpoint)
      {
        boundary_edge_points.has_midpoint = true;
        boundary_edge_points.midpoint = *midpoint;
      }
    };

    const auto addBoundaryPlaneFace = [&](const Point & normal,
                                          const Point & face_centroid,
                                          const dof_id_type previous_node_id,
                                          const dof_id_type next_node_id,
                                          const Point * previous_midpoint,
                                          const Point * next_midpoint,
                                          std::set<std::size_t> * const used_side_plane_indices)
    {
      if (normal.norm() <= primal_boundary_length_tol)
        return;

      const Point unit_normal = normal / normal.norm();
      const Real plane_constant = unit_normal * face_centroid;
      const Real normal_tol = 1e-8;
      std::size_t plane_index = boundary_plane_faces.size();

      for (const auto i : index_range(boundary_plane_faces))
      {
        if (used_side_plane_indices && used_side_plane_indices->count(i))
          continue;

        const Real normal_dot = boundary_plane_faces[i].normal * unit_normal;
        const Real candidate_plane_constant = normal_dot >= 0.0 ? plane_constant : -plane_constant;

        if (std::abs(normal_dot) >= 1.0 - normal_tol &&
            std::abs(boundary_plane_faces[i].plane_constant - candidate_plane_constant) <=
                primal_boundary_length_tol)
        {
          plane_index = i;
          break;
        }
      }

      if (plane_index == boundary_plane_faces.size())
        boundary_plane_faces.push_back({unit_normal, plane_constant, {}, {}});

      if (used_side_plane_indices)
        used_side_plane_indices->insert(plane_index);

      addBoundaryPlaneEdgePoints(
          boundary_plane_faces[plane_index], previous_node_id, face_centroid, previous_midpoint);
      addBoundaryPlaneEdgePoints(
          boundary_plane_faces[plane_index], next_node_id, face_centroid, next_midpoint);
    };

    // Close a boundary face that is open at the primal source vertex through the source, including
    // when the source is collinear with the endpoints.
    const auto closeBoundaryPlaneFaceAtSource = [&](BoundaryPlaneFacePoints3D & boundary_plane_face)
    {
      if (!boundary_vertex_nodes.count(source_node_subdomain_key) ||
          !pointOnBoundaryPlane(boundary_plane_face, source_point))
        return;

      std::vector<Point> endpoints;

      for (const auto & point : boundary_plane_face.face_points.points)
      {
        if (samePoint3D(point, source_point, primal_boundary_length_tol))
          continue;

        unsigned int incident_segments = 0;

        for (const auto & segment : boundary_plane_face.face_points.segments)
          if (samePoint3D(segment.first, point, primal_boundary_length_tol) ||
              samePoint3D(segment.second, point, primal_boundary_length_tol))
            ++incident_segments;

        if (incident_segments == 1)
          addUniquePoint(endpoints, point, primal_boundary_length_tol);
      }

      if (endpoints.size() != 2)
        return;

      addConnectedFaceSegment3D(
          boundary_plane_face.face_points, source_point, endpoints[0], primal_boundary_length_tol);
      addConnectedFaceSegment3D(
          boundary_plane_face.face_points, source_point, endpoints[1], primal_boundary_length_tol);
    };

    for (const auto & elem : node_elems.second)
    {
      const Point elem_centroid = elem->true_centroid();

      for (const auto side : elem->side_index_range())
      {
        auto side_elem = elem->build_side_ptr(side);
        std::vector<dof_id_type> side_node_ids;
        std::vector<Point> side_points;
        side_node_ids.reserve(side_elem->n_vertices());
        side_points.reserve(side_elem->n_vertices());

        // For interior dual elements
        for (const auto n : make_range(side_elem->n_vertices()))
        {
          side_node_ids.push_back(side_elem->node_id(n));
          side_points.push_back(side_elem->point(n));
        }

        const bool side_is_preserved = preservedSide(*elem, side);

        std::pair<dof_id_type, dof_id_type> primal_split_diagonal;
        const bool has_primal_split_diagonal =
            quad4PrimalSplitDiagonal3D(side_node_ids, primal_split_diagonal);

        for (const auto & side_face_part :
             sideFaceParts3D(side_node_ids,
                             side_points,
                             primal_boundary_length_tol,
                             has_primal_split_diagonal ? &primal_split_diagonal : nullptr))
        {
          const auto & current_side_node_ids = side_face_part.node_ids;
          const auto & current_side_points = side_face_part.points;
          const Point face_centroid = sideFacePartDualPoint3D(side_face_part);
          const auto source_node_it =
              std::find(current_side_node_ids.begin(), current_side_node_ids.end(), source_node_id);

          if (source_node_it == current_side_node_ids.end())
            continue;

          const auto source_side_index =
              cast_int<unsigned int>(source_node_it - current_side_node_ids.begin());
          const dof_id_type previous_node_id =
              current_side_node_ids[(source_side_index + current_side_node_ids.size() - 1) %
                                    current_side_node_ids.size()];
          const dof_id_type next_node_id =
              current_side_node_ids[(source_side_index + 1) % current_side_node_ids.size()];

          const auto previous_edge_key = edgeKey(source_node_id, previous_node_id);
          const auto next_edge_key = edgeKey(source_node_id, next_node_id);
          auto & previous_edge_points = edge_to_points[previous_edge_key];
          auto & next_edge_points = edge_to_points[next_edge_key];

          addConnectedFacePoint3D(previous_edge_points, elem_centroid, primal_boundary_length_tol);
          addConnectedFacePoint3D(next_edge_points, elem_centroid, primal_boundary_length_tol);

          if (!side_is_preserved)
          {
            const Point neighbor_centroid = elem->neighbor_ptr(side)->true_centroid();

            addConnectedFaceSegment3D(
                previous_edge_points, elem_centroid, neighbor_centroid, primal_boundary_length_tol);
            addConnectedFaceSegment3D(
                next_edge_points, elem_centroid, neighbor_centroid, primal_boundary_length_tol);
            continue;
          }

          // Only preserved boundary faces are routed through their face-part dual point.
          addConnectedFaceSegment3D(
              previous_edge_points, elem_centroid, face_centroid, primal_boundary_length_tol);
          addConnectedFaceSegment3D(
              next_edge_points, elem_centroid, face_centroid, primal_boundary_length_tol);

          if (side_is_preserved)
          {
            const auto previous_boundary_edge_key =
                EdgeSubdomainKey{previous_edge_key, source_subdomain_key};
            const auto next_boundary_edge_key =
                EdgeSubdomainKey{next_edge_key, source_subdomain_key};

            const Point * const previous_midpoint =
                boundaryEdgeMidpoint(previous_boundary_edge_key);
            const Point * const next_midpoint = boundaryEdgeMidpoint(next_boundary_edge_key);

            if (boundary_vertex_nodes.count(source_node_subdomain_key))
            {
              if (previous_midpoint)
              {
                addConnectedFaceSegment3D(previous_edge_points,
                                          face_centroid,
                                          *previous_midpoint,
                                          primal_boundary_length_tol);
              }

              if (next_midpoint)
              {
                addConnectedFaceSegment3D(
                    next_edge_points, face_centroid, *next_midpoint, primal_boundary_length_tol);
              }
            }

            // Keep the two planes of this twisted QUAD separate while allowing either plane to
            // merge with a coplanar triangle from an adjacent boundary face.
            if (current_side_points.size() == 4 && side_face_part.has_selected_diagonal)
            {
              std::set<std::size_t> used_side_plane_indices;
              const auto boundaryPartMidpoint =
                  [&](const dof_id_type other_node_id) -> const Point *
              {
                const auto edge = edgeKey(source_node_id, other_node_id);

                if (edge == side_face_part.selected_diagonal)
                  return nullptr;

                return boundaryEdgeMidpoint(EdgeSubdomainKey{edge, source_subdomain_key});
              };

              for (const auto & point_indices :
                   sideFacePartTriangleIndices3D(side_face_part, primal_boundary_length_tol))
              {
                std::size_t source_part_index = point_indices.size();

                for (const auto part_index : index_range(point_indices))
                  if (current_side_node_ids[point_indices[part_index]] == source_node_id)
                  {
                    source_part_index = part_index;
                    break;
                  }

                if (source_part_index == point_indices.size())
                  continue;

                const dof_id_type previous_part_node_id =
                    current_side_node_ids[point_indices[(source_part_index + 2) %
                                                        point_indices.size()]];
                const dof_id_type next_part_node_id =
                    current_side_node_ids[point_indices[(source_part_index + 1) %
                                                        point_indices.size()]];
                Point normal =
                    (current_side_points[point_indices[1]] - current_side_points[point_indices[0]])
                        .cross(current_side_points[point_indices[2]] -
                               current_side_points[point_indices[0]]);

                if (normal.norm() <= 1e-12)
                  continue;

                if (normal * (elem_centroid - face_centroid) > 0.0)
                  normal = -1.0 * normal;

                addBoundaryPlaneFace(normal,
                                     face_centroid,
                                     previous_part_node_id,
                                     next_part_node_id,
                                     boundaryPartMidpoint(previous_part_node_id),
                                     boundaryPartMidpoint(next_part_node_id),
                                     &used_side_plane_indices);
              }
            }
            else
            {
              Point normal = faceNormal3D(current_side_points);

              if (normal.norm() > 1e-12)
              {
                if (normal * (elem_centroid - face_centroid) > 0.0)
                  normal = -1.0 * normal;

                addBoundaryPlaneFace(normal,
                                     face_centroid,
                                     previous_node_id,
                                     next_node_id,
                                     previous_midpoint,
                                     next_midpoint,
                                     nullptr);
              }
            }
          }
        }
      }
    }

    // Preserve this candidate for direct construction and each configured concave treatment.
    std::vector<std::vector<Point>> polycut_side_points;
    std::vector<PrimalEdgeSide3D> primal_edge_sides;

    for (const auto & edge_points : edge_to_points)
    {
      if (edge_points.second.points.size() < 3)
        continue;

      const dof_id_type other_node_id = edge_points.first.first == source_node_id
                                            ? edge_points.first.second
                                            : edge_points.first.first;
      const Point edge_axis = *input_mesh->node_ptr(other_node_id) - source_point;
      const auto sorted_edge_points =
          sortConnectedFacePoints3D(edge_points.second, edge_axis, primal_boundary_length_tol);
      const std::size_t previous_side_count = polycut_side_points.size();

      addSidePoints3D(polycut_side_points, sorted_edge_points);

      if (polycut_side_points.size() != previous_side_count)
        primal_edge_sides.push_back({EdgeSubdomainKey{edge_points.first, source_subdomain_key},
                                     previous_side_count,
                                     0.5 * (source_point + *input_mesh->node_ptr(other_node_id)),
                                     false});
    }

    // Coplanar boundary parts that meet only at the source are separate faces. Partition each
    // geometric plane using its source-edge/face-centroid incidence before closing the parts.
    const auto boundaryPlaneEdgeComponents =
        [&](const BoundaryPlaneFacePoints3D & boundary_plane_face)
    {
      std::vector<std::vector<std::size_t>> components;
      std::vector<bool> visited(boundary_plane_face.edge_points.size(), false);

      const auto edgePointsShareFaceCentroid = [&](const BoundaryPlaneEdgePoints3D & edge_points0,
                                                   const BoundaryPlaneEdgePoints3D & edge_points1)
      {
        for (const auto & face_centroid0 : edge_points0.face_centroids)
          for (const auto & face_centroid1 : edge_points1.face_centroids)
            if (samePoint3D(face_centroid0, face_centroid1, primal_boundary_length_tol))
              return true;

        return false;
      };

      for (const auto start_index : index_range(boundary_plane_face.edge_points))
      {
        if (visited[start_index])
          continue;

        std::vector<std::size_t> component;
        std::vector<std::size_t> pending{start_index};
        visited[start_index] = true;

        while (!pending.empty())
        {
          const std::size_t edge_index = pending.back();
          pending.pop_back();
          component.push_back(edge_index);

          for (const auto candidate_index : index_range(boundary_plane_face.edge_points))
            if (!visited[candidate_index] &&
                edgePointsShareFaceCentroid(boundary_plane_face.edge_points[edge_index],
                                            boundary_plane_face.edge_points[candidate_index]))
            {
              visited[candidate_index] = true;
              pending.push_back(candidate_index);
            }
        }

        components.push_back(std::move(component));
      }

      return components;
    };

    for (const auto & boundary_plane_face : boundary_plane_faces)
    {
      for (const auto & component : boundaryPlaneEdgeComponents(boundary_plane_face))
      {
        BoundaryPlaneFacePoints3D boundary_plane_component{
            boundary_plane_face.normal, boundary_plane_face.plane_constant, {}, {}};

        for (const auto edge_index : component)
        {
          const auto & boundary_edge_points = boundary_plane_face.edge_points[edge_index];

          if (boundary_edge_points.has_midpoint)
            for (const auto & face_centroid : boundary_edge_points.face_centroids)
              addConnectedFaceSegment3D(boundary_plane_component.face_points,
                                        face_centroid,
                                        boundary_edge_points.midpoint,
                                        primal_boundary_length_tol);
          else if (boundary_edge_points.face_centroids.size() == 2)
            addConnectedFaceSegment3D(boundary_plane_component.face_points,
                                      boundary_edge_points.face_centroids[0],
                                      boundary_edge_points.face_centroids[1],
                                      primal_boundary_length_tol);
          else
            for (const auto & face_centroid : boundary_edge_points.face_centroids)
              addConnectedFacePoint3D(
                  boundary_plane_component.face_points, face_centroid, primal_boundary_length_tol);
        }

        closeBoundaryPlaneFaceAtSource(boundary_plane_component);

        addSidePoints3D(polycut_side_points,
                        sortConnectedFacePoints3D(boundary_plane_component.face_points,
                                                  boundary_plane_component.normal,
                                                  primal_boundary_length_tol),
                        primal_boundary_length_tol);
      }
    }

    if (polycut_side_points.size() < 4)
      continue;

    const bool has_concave_boundary_normals = hasConcaveBoundaryNormals(source_node_subdomain_key);
    PolyCutEdge3D concave_edge;
    const bool searched_concave_edge = use_split || has_concave_boundary_normals;
    const Real concavity_tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real concavity_polyhedron_scale = polyhedronScale3D(polycut_side_points);
    const Real concavity_length_tol = concavity_tol * concavity_polyhedron_scale;
    const bool has_concave_edge =
        searched_concave_edge &&
        findConcavePolyhedronEdge3D(
            polycut_side_points, boundary_normal_dot_tol, concavity_length_tol, concave_edge);
    // C0Polyhedron construction does not reject this boundary-plane configuration reliably.
    const bool has_nonconvex_face_plane =
        !has_concave_edge && !boundary_plane_faces.empty() &&
        hasPointOutsidePolyhedronFacePlanes3D(polycut_side_points, concavity_length_tol);
    const bool force_tetrahedralize =
        (has_concave_boundary_normals && has_concave_edge) || has_nonconvex_face_plane;
    if (buffer_dual_cells)
      split_dual_cell_side_points.push_back({std::move(polycut_side_points),
                                             source_node_id,
                                             source_subdomain_key,
                                             searched_concave_edge,
                                             has_concave_edge,
                                             concave_edge,
                                             force_tetrahedralize,
                                             false,
                                             {},
                                             false,
                                             {},
                                             std::move(primal_edge_sides),
                                             node_elems.second});
    else
      addPolyhedronOrTetrahedralize(polycut_side_points,
                                    force_tetrahedralize,
                                    searched_concave_edge,
                                    has_concave_edge,
                                    source_node_id,
                                    source_subdomain_key,
                                    node_elems.second,
                                    nullptr);
  }

  if (buffer_dual_cells)
  {
    enum class SurfaceConformance3D
    {
      unavailable,
      conforming,
      nonconforming
    };

    const auto orientedSurfaceTriangles = [&](const std::vector<std::vector<Point>> & side_points,
                                              const SubdomainID output_subdomain_id,
                                              const std::vector<const Elem *> & primal_elems,
                                              std::vector<std::vector<Point>> & surface_triangles)
    {
      const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
      const Real polyhedron_scale = polyhedronScale3D(side_points);
      const Real length_tol = tol * polyhedron_scale;
      bool touches_preserved_primal_boundary = false;

      if (_preserve_diagonals)
        for (const auto * const elem : primal_elems)
        {
          for (const auto side : elem->side_index_range())
            if (preservedSide(*elem, side))
            {
              touches_preserved_primal_boundary = true;
              break;
            }

          if (touches_preserved_primal_boundary)
            break;
        }

      const auto validSurfaceSegment = [&](const Point & point0, const Point & point1)
      {
        return !_preserve_diagonals || !touches_preserved_primal_boundary ||
               segmentInsidePrimalBoundary(output_subdomain_id, point0, point1);
      };

      if (!surfaceTriangles3D(side_points,
                              surface_triangles,
                              validSurfaceSegment,
                              length_tol,
                              &dual_point_sources) ||
          !orientSurfaceTriangles3D(surface_triangles, length_tol))
        return false;

      return true;
    };

    const auto surfaceConformance = [&](const std::vector<std::vector<Point>> & side_points,
                                        const SubdomainID output_subdomain_id,
                                        const std::vector<const Elem *> & primal_elems)
    {
      const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
      const Real polyhedron_scale = polyhedronScale3D(side_points);
      const Real length_tol = tol * polyhedron_scale;
      std::vector<std::vector<Point>> surface_triangles;

      if (!orientedSurfaceTriangles(
              side_points, output_subdomain_id, primal_elems, surface_triangles))
        return SurfaceConformance3D::unavailable;

      return surfaceTrianglesAreConforming3D(surface_triangles, length_tol)
                 ? SurfaceConformance3D::conforming
                 : SurfaceConformance3D::nonconforming;
    };

    const auto isBodyCentroidRing = [&](const std::vector<Point> & side_points)
    {
      if (side_points.size() < 4)
        return false;

      for (const auto & point : side_points)
      {
        const auto * const point_sources = findDualPointSources3D(dual_point_sources, point);

        if (!point_sources || point_sources->size() != 1 ||
            !point_sources->count(DualPointSource3D::body_centroid))
          return false;
      }

      return true;
    };

    const auto replacePrimalEdgeSideWithFan = [&](std::vector<std::vector<Point>> & side_points,
                                                  const PrimalEdgeSide3D & primal_edge_side)
    {
      if (primal_edge_side.replaced || primal_edge_side.side_index >= side_points.size())
        return false;

      const auto & replaced_side = side_points[primal_edge_side.side_index];
      std::vector<std::vector<Point>> replaced_side_points;
      replaced_side_points.reserve(side_points.size() + replaced_side.size() - 1);

      for (const auto side_index : index_range(side_points))
        if (side_index != primal_edge_side.side_index)
          replaced_side_points.push_back(side_points[side_index]);
        else
        {
          const std::size_t previous_side_count = replaced_side_points.size();

          for (const auto i : index_range(replaced_side))
            addSidePoints3D(replaced_side_points,
                            {primal_edge_side.midpoint,
                             replaced_side[i],
                             replaced_side[(i + 1) % replaced_side.size()]},
                            primal_boundary_length_tol);

          if (replaced_side_points.size() != previous_side_count + replaced_side.size())
            return false;
        }

      side_points.swap(replaced_side_points);
      return true;
    };

    if (use_netgen)
    {
      using PrimalEdgeSideLocation3D = std::pair<std::size_t, std::size_t>;
      std::map<EdgeSubdomainKey, std::vector<PrimalEdgeSideLocation3D>> edge_side_locations;

      for (const auto cell_index : index_range(split_dual_cell_side_points))
        for (const auto side_index :
             index_range(split_dual_cell_side_points[cell_index].primal_edge_sides))
          edge_side_locations
              [split_dual_cell_side_points[cell_index].primal_edge_sides[side_index].edge_key]
                  .push_back({cell_index, side_index});

      std::set<EdgeSubdomainKey> repaired_primal_edges;

      const auto nonNetgenTreatmentCanBuild = [&](const SplitDualCellSidePoints3D & dual_cell)
      {
        const bool concave_edge_search_failed =
            dual_cell.searched_concave_edge && !dual_cell.has_concave_edge;

        for (const auto & concave_treatment : _concave_treatment)
          if (concave_treatment == "split")
          {
            if (concave_edge_search_failed || !dual_cell.has_concave_edge)
              continue;

            SplitCutFaceCandidate3D split_plan;

            if (findValidSplitPlan(
                    dual_cell.polycut_side_points, dual_cell.concave_edge, split_plan))
              return true;
          }
          else if (concave_treatment == "polycut")
          {
            if (concave_edge_search_failed)
              continue;

            const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
            const Real polyhedron_scale = polyhedronScale3D(dual_cell.polycut_side_points);
            const Real length_tol = tol * polyhedron_scale;

            for (const auto & polycut_candidate : polyCutSidePointCandidates3D(
                     dual_cell.polycut_side_points, boundary_normal_dot_tol, length_tol))
            {
              auto validation_mesh = buildReplicatedMesh(3);

              if (canBuildPolyhedron(
                      *validation_mesh, polycut_candidate.child0_side_points, false, true, true) &&
                  canBuildPolyhedron(
                      *validation_mesh, polycut_candidate.child1_side_points, false, true, true))
                return true;
            }
          }

        return false;
      };

      const auto directTreatmentCanBuild = [&](const SplitDualCellSidePoints3D & dual_cell)
      {
        if (dual_cell.force_tetrahedralize || dual_cell.has_concave_edge)
          return false;

        auto validation_mesh = buildReplicatedMesh(3);
        return canBuildPolyhedron(
            *validation_mesh, dual_cell.polycut_side_points, true, false, false);
      };

      const auto repairBoundaryChord = [&](const std::size_t target_cell_index)
      {
        auto & target_cell = split_dual_cell_side_points[target_cell_index];
        const auto target_node_key =
            NodeSubdomainKey{target_cell.source_node_id, target_cell.output_subdomain_id};

        if (target_cell.has_boundary_chord_split_plan ||
            !boundary_vertex_nodes.count(target_node_key) ||
            !primal_boundary_surface_triangles.count(target_cell.output_subdomain_id))
          return false;

        const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
        const Real polyhedron_scale = polyhedronScale3D(target_cell.polycut_side_points);
        const Real length_tol = tol * polyhedron_scale;
        const Point source_point = *input_mesh->node_ptr(target_cell.source_node_id);
        std::vector<std::vector<Point>> parent_surface_triangles;

        if (!orientedSurfaceTriangles(target_cell.polycut_side_points,
                                      target_cell.output_subdomain_id,
                                      target_cell.primal_elems,
                                      parent_surface_triangles))
          return false;

        const auto chordTriangles = [&](const std::vector<Point> & body_ring,
                                        const Point & chord_point0,
                                        const Point & chord_point1,
                                        Point & midpoint,
                                        std::array<std::size_t, 2> & triangle_indices)
        {
          unsigned int incident_triangles = 0;
          triangle_indices.fill(parent_surface_triangles.size());

          for (const auto triangle_index : index_range(parent_surface_triangles))
          {
            const auto & triangle = parent_surface_triangles[triangle_index];

            if (!faceContainsEdge3D(triangle, chord_point0, chord_point1, length_tol))
              continue;

            ++incident_triangles;
            const Point * third_point = nullptr;

            for (const auto & point : triangle)
              if (!samePoint3D(point, chord_point0, length_tol) &&
                  !samePoint3D(point, chord_point1, length_tol))
              {
                if (third_point)
                  return false;

                third_point = &point;
              }

            if (!third_point)
              return false;

            if (faceContainsPoint3D(body_ring, *third_point, length_tol))
            {
              if (triangle_indices[0] != parent_surface_triangles.size())
                return false;

              triangle_indices[0] = triangle_index;
              continue;
            }

            const auto * const point_sources =
                findDualPointSources3D(dual_point_sources, *third_point);

            if (!point_sources ||
                !point_sources->count(DualPointSource3D::boundary_edge_midpoint) ||
                triangle_indices[1] != parent_surface_triangles.size())
              return false;

            midpoint = *third_point;
            triangle_indices[1] = triangle_index;
          }

          return incident_triangles == 2 &&
                 triangle_indices[0] != parent_surface_triangles.size() &&
                 triangle_indices[1] != parent_surface_triangles.size();
        };

        for (const auto & central_edge_side : target_cell.primal_edge_sides)
        {
          if (central_edge_side.replaced ||
              central_edge_side.side_index >= target_cell.polycut_side_points.size())
            continue;

          const auto & body_ring = target_cell.polycut_side_points[central_edge_side.side_index];

          if (body_ring.size() != 4 || !isBodyCentroidRing(body_ring))
            continue;

          std::vector<Point> unique_body_points;

          for (const auto & body_point : body_ring)
            addUniquePoint(unique_body_points, body_point, length_tol);

          if (unique_body_points.size() != body_ring.size())
            continue;

          // Match the offending triangles exactly below. Sampled segment containment can miss
          // thin boundary excursions and must not exclude this repair.
          // A four-point ring has only two distinct pairs of opposite edges.
          for (const auto bad_chord_index : make_range(std::size_t(2)))
          {
            const Point & body_b = body_ring[bad_chord_index];
            const Point & body_c = body_ring[(bad_chord_index + 1) % body_ring.size()];
            const Point & body_d = body_ring[(bad_chord_index + 2) % body_ring.size()];
            const Point & body_a = body_ring[(bad_chord_index + 3) % body_ring.size()];
            std::array<Point, 2> boundary_midpoints;
            std::array<std::array<std::size_t, 2>, 2> chord_triangle_indices;

            if (!chordTriangles(
                    body_ring, body_b, body_c, boundary_midpoints[0], chord_triangle_indices[0]) ||
                !chordTriangles(
                    body_ring, body_d, body_a, boundary_midpoints[1], chord_triangle_indices[1]))
              continue;

            const Point & boundary_midpoint0 = boundary_midpoints[0];
            const Point & boundary_midpoint1 = boundary_midpoints[1];
            bool distinct_split_points =
                !samePoint3D(source_point, boundary_midpoint0, length_tol) &&
                !samePoint3D(source_point, boundary_midpoint1, length_tol) &&
                !samePoint3D(boundary_midpoint0, boundary_midpoint1, length_tol);

            for (const auto & body_point : body_ring)
              if (samePoint3D(source_point, body_point, length_tol) ||
                  samePoint3D(boundary_midpoint0, body_point, length_tol) ||
                  samePoint3D(boundary_midpoint1, body_point, length_tol))
              {
                distinct_split_points = false;
                break;
              }

            if (!distinct_split_points)
              continue;

            // Remove the actual body-side and boundary-side triangle incident on each chord. This
            // is independent of the body ring direction and the quad's selected diagonal.
            const std::vector<std::size_t> removed_triangle_indices = {
                chord_triangle_indices[0][0],
                chord_triangle_indices[0][1],
                chord_triangle_indices[1][0],
                chord_triangle_indices[1][1]};
            bool found_removed_triangles = true;

            for (const auto i : index_range(removed_triangle_indices))
              for (const auto j : make_range(i + 1, removed_triangle_indices.size()))
                if (removed_triangle_indices[i] == removed_triangle_indices[j])
                  found_removed_triangles = false;

            if (!found_removed_triangles)
              continue;

            std::vector<std::vector<Point>> remaining_triangles;
            remaining_triangles.reserve(parent_surface_triangles.size() -
                                        removed_triangle_indices.size());

            for (const auto triangle_index : index_range(parent_surface_triangles))
              if (std::find(removed_triangle_indices.begin(),
                            removed_triangle_indices.end(),
                            triangle_index) == removed_triangle_indices.end())
                remaining_triangles.push_back(parent_surface_triangles[triangle_index]);

            const auto isSeamEdge = [&](const Point & point0, const Point & point1)
            {
              return sameSegment3D(point0, point1, boundary_midpoint0, source_point, length_tol) ||
                     sameSegment3D(point0, point1, source_point, boundary_midpoint1, length_tol);
            };
            const auto trianglesShareNonSeamEdge =
                [&](const std::vector<Point> & triangle0, const std::vector<Point> & triangle1)
            {
              for (const auto edge0 : make_range(std::size_t(3)))
                for (const auto edge1 : make_range(std::size_t(3)))
                  if (sameSegment3D(triangle0[edge0],
                                    triangle0[(edge0 + 1) % triangle0.size()],
                                    triangle1[edge1],
                                    triangle1[(edge1 + 1) % triangle1.size()],
                                    length_tol) &&
                      !isSeamEdge(triangle0[edge0], triangle0[(edge0 + 1) % triangle0.size()]))
                    return true;

              return false;
            };

            std::vector<unsigned int> triangle_components(remaining_triangles.size(),
                                                          std::numeric_limits<unsigned int>::max());
            unsigned int component_count = 0;

            for (const auto start_triangle : index_range(remaining_triangles))
            {
              if (triangle_components[start_triangle] != std::numeric_limits<unsigned int>::max())
                continue;

              std::vector<std::size_t> pending{start_triangle};
              triangle_components[start_triangle] = component_count;

              while (!pending.empty())
              {
                const std::size_t triangle_index = pending.back();
                pending.pop_back();

                for (const auto candidate_triangle : index_range(remaining_triangles))
                  if (triangle_components[candidate_triangle] ==
                          std::numeric_limits<unsigned int>::max() &&
                      trianglesShareNonSeamEdge(remaining_triangles[triangle_index],
                                                remaining_triangles[candidate_triangle]))
                  {
                    triangle_components[candidate_triangle] = component_count;
                    pending.push_back(candidate_triangle);
                  }
              }

              ++component_count;
            }

            if (component_count != 2)
              continue;

            std::array<std::vector<std::vector<Point>>, 2> components;

            for (const auto triangle_index : index_range(remaining_triangles))
              components[triangle_components[triangle_index]].push_back(
                  remaining_triangles[triangle_index]);

            const auto surfaceEdgeCount =
                [&](const std::vector<std::vector<Point>> & surface_triangles,
                    const Point & point0,
                    const Point & point1)
            {
              unsigned int count = 0;

              for (const auto & triangle : surface_triangles)
                if (faceContainsEdge3D(triangle, point0, point1, length_tol))
                  ++count;

              return count;
            };
            const auto hasBoundaryPath = [&](const std::vector<std::vector<Point>> & component,
                                             const Point & midpoint_body_point,
                                             const Point & other_body_point)
            {
              return surfaceEdgeCount(component, boundary_midpoint0, source_point) == 1 &&
                     surfaceEdgeCount(component, source_point, boundary_midpoint1) == 1 &&
                     surfaceEdgeCount(component, boundary_midpoint0, midpoint_body_point) == 1 &&
                     surfaceEdgeCount(component, midpoint_body_point, other_body_point) == 1 &&
                     surfaceEdgeCount(component, other_body_point, boundary_midpoint1) == 1;
            };

            if (hasBoundaryPath(components[1], body_b, body_a) &&
                hasBoundaryPath(components[0], body_c, body_d))
              std::swap(components[0], components[1]);
            else if (!(hasBoundaryPath(components[0], body_b, body_a) &&
                       hasBoundaryPath(components[1], body_c, body_d)))
              continue;

            // Close the components on midpoint0-source-midpoint1-A-B and
            // midpoint0-source-midpoint1-D-C.
            std::vector<std::vector<Point>> child0_cap = {
                {source_point, boundary_midpoint0, body_b},
                {source_point, body_b, body_a},
                {source_point, body_a, boundary_midpoint1}};
            std::vector<std::vector<Point>> child1_cap = {
                {source_point, boundary_midpoint1, body_d},
                {source_point, body_d, body_c},
                {source_point, body_c, boundary_midpoint0}};
            const auto validCap = [&](const std::vector<std::vector<Point>> & cap)
            {
              for (const auto & triangle : cap)
                if (!hasNonzeroArea3D(triangle, length_tol * length_tol))
                  return false;

              return true;
            };

            if (!validCap(child0_cap) || !validCap(child1_cap))
              continue;

            SplitCutFaceCandidate3D split_plan;
            split_plan.child0_side_points = std::move(components[0]);
            split_plan.child1_side_points = std::move(components[1]);
            split_plan.child0_side_points.insert(
                split_plan.child0_side_points.end(), child0_cap.begin(), child0_cap.end());
            split_plan.child1_side_points.insert(
                split_plan.child1_side_points.end(), child1_cap.begin(), child1_cap.end());

            if (!orientSurfaceTriangles3D(split_plan.child0_side_points, length_tol) ||
                !orientSurfaceTriangles3D(split_plan.child1_side_points, length_tol) ||
                !surfaceTrianglesAreConforming3D(split_plan.child0_side_points, length_tol) ||
                !surfaceTrianglesAreConforming3D(split_plan.child1_side_points, length_tol))
              continue;

            target_cell.has_boundary_chord_split_plan = true;
            target_cell.boundary_chord_split_plan = std::move(split_plan);
            return true;
          }
        }

        return false;
      };

      for (const auto cell_index : index_range(split_dual_cell_side_points))
      {
        auto & dual_cell = split_dual_cell_side_points[cell_index];

        // Apply the exact boundary-chord repair before another treatment accepts the uncut cell.
        if (surfaceConformance(dual_cell.polycut_side_points,
                               dual_cell.output_subdomain_id,
                               dual_cell.primal_elems) != SurfaceConformance3D::nonconforming)
          continue;

        if (repairBoundaryChord(cell_index))
          continue;

        if (directTreatmentCanBuild(dual_cell) || nonNetgenTreatmentCanBuild(dual_cell))
          continue;

        for (const auto primal_edge_side_index : index_range(dual_cell.primal_edge_sides))
        {
          const auto & primal_edge_side = dual_cell.primal_edge_sides[primal_edge_side_index];

          if (repaired_primal_edges.count(primal_edge_side.edge_key) || primal_edge_side.replaced ||
              primal_edge_side.side_index >= dual_cell.polycut_side_points.size() ||
              !isBodyCentroidRing(dual_cell.polycut_side_points[primal_edge_side.side_index]))
            continue;

          const auto locations_it = edge_side_locations.find(primal_edge_side.edge_key);

          // A primal-edge dual face is shared by the dual cells at its two endpoints.
          if (locations_it == edge_side_locations.end() || locations_it->second.size() != 2)
            continue;

          std::vector<std::size_t> affected_cell_indices;
          std::vector<std::size_t> affected_edge_side_indices;
          std::vector<std::size_t> replaced_side_indices;
          std::vector<std::size_t> side_index_offsets;
          std::vector<std::vector<std::vector<Point>>> candidate_cell_side_points;
          bool valid_repair = true;

          for (const auto & location : locations_it->second)
          {
            const auto & affected_cell = split_dual_cell_side_points[location.first];

            if (affected_cell.has_boundary_chord_split_plan ||
                std::find(affected_cell_indices.begin(),
                          affected_cell_indices.end(),
                          location.first) != affected_cell_indices.end())
            {
              valid_repair = false;
              break;
            }

            const auto & affected_edge_side = affected_cell.primal_edge_sides[location.second];
            auto candidate_side_points = affected_cell.polycut_side_points;

            if (affected_edge_side.replaced ||
                affected_edge_side.side_index >= affected_cell.polycut_side_points.size() ||
                !isBodyCentroidRing(
                    affected_cell.polycut_side_points[affected_edge_side.side_index]) ||
                !replacePrimalEdgeSideWithFan(candidate_side_points, affected_edge_side) ||
                surfaceConformance(candidate_side_points,
                                   affected_cell.output_subdomain_id,
                                   affected_cell.primal_elems) != SurfaceConformance3D::conforming)
            {
              valid_repair = false;
              break;
            }

            affected_cell_indices.push_back(location.first);
            affected_edge_side_indices.push_back(location.second);
            replaced_side_indices.push_back(affected_edge_side.side_index);
            side_index_offsets.push_back(
                affected_cell.polycut_side_points[affected_edge_side.side_index].size() - 1);
            candidate_cell_side_points.push_back(std::move(candidate_side_points));
          }

          if (!valid_repair)
            continue;

          for (const auto candidate_index : index_range(affected_cell_indices))
          {
            auto & affected_cell =
                split_dual_cell_side_points[affected_cell_indices[candidate_index]];
            affected_cell.polycut_side_points =
                std::move(candidate_cell_side_points[candidate_index]);

            for (auto & recorded_edge_side : affected_cell.primal_edge_sides)
              if (!recorded_edge_side.replaced &&
                  recorded_edge_side.side_index > replaced_side_indices[candidate_index])
                recorded_edge_side.side_index += side_index_offsets[candidate_index];

            affected_cell.primal_edge_sides[affected_edge_side_indices[candidate_index]].replaced =
                true;
          }

          repaired_primal_edges.insert(primal_edge_side.edge_key);
          break;
        }
      }
    }

    std::vector<SplitEdgePoint3D> split_edge_points;
    std::vector<SplitFaceReplacement3D> split_face_replacements;

    for (auto & split_dual_cell_side_point : split_dual_cell_side_points)
    {
      if (!use_split || split_dual_cell_side_point.has_boundary_chord_split_plan ||
          !split_dual_cell_side_point.has_concave_edge)
        continue;

      if (findValidSplitPlan(split_dual_cell_side_point.polycut_side_points,
                             split_dual_cell_side_point.concave_edge,
                             split_dual_cell_side_point.split_plan))
        split_dual_cell_side_point.has_split_plan = true;

      if (!split_dual_cell_side_point.has_split_plan)
        continue;

      for (const auto & split_edge_point : split_dual_cell_side_point.split_plan.split_edge_points)
        addUniqueSplitEdgePoint3D(split_edge_points, split_edge_point, primal_boundary_length_tol);

      for (const auto & face_replacement :
           split_dual_cell_side_point.split_plan.split_face_replacements)
        addUniqueSplitFaceReplacement3D(
            split_face_replacements, face_replacement, primal_boundary_length_tol);
    }

    const auto split_point_index = buildSplitPointIndex3D(
        split_edge_points, split_face_replacements, primal_boundary_length_tol);

    for (auto & split_dual_cell_side_point : split_dual_cell_side_points)
    {
      insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.polycut_side_points,
                                     split_edge_points,
                                     split_face_replacements,
                                     split_point_index);

      if (split_dual_cell_side_point.has_split_plan)
      {
        insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.split_plan.child0_side_points,
                                       split_edge_points,
                                       split_face_replacements,
                                       split_point_index);
        insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.split_plan.child1_side_points,
                                       split_edge_points,
                                       split_face_replacements,
                                       split_point_index);
      }

      if (split_dual_cell_side_point.has_boundary_chord_split_plan)
      {
        insertSplitPointsOnSideEdges3D(
            split_dual_cell_side_point.boundary_chord_split_plan.child0_side_points,
            split_edge_points,
            split_face_replacements,
            split_point_index);
        insertSplitPointsOnSideEdges3D(
            split_dual_cell_side_point.boundary_chord_split_plan.child1_side_points,
            split_edge_points,
            split_face_replacements,
            split_point_index);
      }

      if (split_dual_cell_side_point.has_boundary_chord_split_plan)
      {
        const auto boundary_info = primalBoundaryInfo(split_dual_cell_side_point.primal_elems,
                                                      split_dual_cell_side_point.source_node_id);
        const auto addBoundaryChordChild = [&](const std::vector<std::vector<Point>> & side_points)
        {
          auto validation_mesh = buildReplicatedMesh(3);

          // The shared surface is already final. A late split or cut could introduce exterior
          // points that were not propagated to neighboring cells.
          if (canBuildPolyhedron(*validation_mesh, side_points, false, true, true) &&
              addPolyhedron(side_points,
                            split_dual_cell_side_point.output_subdomain_id,
                            boundary_info,
                            false,
                            true))
            return;

          if (!addTetrahedralizedPolyhedron(
                  side_points, split_dual_cell_side_point.output_subdomain_id, true, boundary_info))
            mooseError("Could not tetrahedralize a boundary-chord split 3D dual polyhedron child.");
        };

        addBoundaryChordChild(
            split_dual_cell_side_point.boundary_chord_split_plan.child0_side_points);
        addBoundaryChordChild(
            split_dual_cell_side_point.boundary_chord_split_plan.child1_side_points);

        continue;
      }

      addPolyhedronOrTetrahedralize(split_dual_cell_side_point.polycut_side_points,
                                    split_dual_cell_side_point.force_tetrahedralize,
                                    split_dual_cell_side_point.searched_concave_edge,
                                    split_dual_cell_side_point.has_concave_edge,
                                    split_dual_cell_side_point.source_node_id,
                                    split_dual_cell_side_point.output_subdomain_id,
                                    split_dual_cell_side_point.primal_elems,
                                    split_dual_cell_side_point.has_split_plan
                                        ? &split_dual_cell_side_point.split_plan
                                        : nullptr);
    }
  }

  dualMesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(dualMesh);
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate2D(std::unique_ptr<MeshBase> input_mesh)
{
  const bool use_voronoi = _dual_mesh_type == "voronoi";

  if (use_voronoi && !_preserve_primal_subdomains.empty())
    paramError("preserve_primal_subdomains",
               "Preserving primal subdomains is currently only implemented for barycentric 2D "
               "duals.");

  const auto preserve_primal_subdomain_ids = preservedPrimalSubdomainIDs(*input_mesh);
  const BoundaryInfo & input_boundary_info = input_mesh->get_boundary_info();

  // More subdomain helpers, these are 2D-specific
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
          std::vector<boundary_id_type> boundary_ids;
          input_boundary_info.boundary_ids(elem, side, boundary_ids);
          std::sort(boundary_ids.begin(), boundary_ids.end());
          boundary_ids.erase(std::unique(boundary_ids.begin(), boundary_ids.end()),
                             boundary_ids.end());

          boundary_region.preserved_boundary_segments.push_back(
              {node0, node1, side_elem->point(0), side_elem->point(1), boundary_ids});

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

      if (boundary_region.preserved_boundary_segments[segment_ids[0]].boundary_ids !=
          boundary_region.preserved_boundary_segments[segment_ids[1]].boundary_ids)
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
      if (pointInPolygon2D(point, boundary_point_loop))
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
      node_elem->set_node(0, new_node);
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

    big_square->set_node(0, p0);
    big_square->set_node(1, p1);
    big_square->set_node(2, p2);
    big_square->set_node(3, p3);

    tri_mesh->add_elem(std::move(big_square));

    // The triangulator has no shortage of input parameters. This configuration does best.
    Poly2TriTriangulator triangulator(dynamic_cast<UnstructuredMesh &>(*tri_mesh));
    triangulator.triangulation_type() = libMesh::TriangulatorInterface::PSLG;
    triangulator.minimum_angle() = 0;
    triangulator.desired_area() = 0;

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
  BoundaryInfo & dual_boundary_info = dualMesh->get_boundary_info();

  dual_boundary_info.set_sideset_name_map() = input_boundary_info.get_sideset_name_map();
  dual_boundary_info.set_nodeset_name_map() = input_boundary_info.get_nodeset_name_map();

  std::map<RoundedPointKey2D, std::vector<Node *>> dual_nodes_by_key;
  const Real dual_node_tol = std::max(length_tol, Real(1e-12));
  const auto roundedPointKey = [&](const Point & point)
  {
    return RoundedPointKey2D{{static_cast<long long>(std::llround(point(0) / dual_node_tol)),
                              static_cast<long long>(std::llround(point(1) / dual_node_tol))}};
  };

  struct PrimalNodeBoundaryIDs2D
  {
    Point point;
    std::vector<boundary_id_type> boundary_ids;
  };

  std::map<RoundedPointKey2D, std::vector<PrimalNodeBoundaryIDs2D>> primal_node_boundary_ids;

  for (const auto * const node : input_mesh->node_ptr_range())
  {
    std::vector<boundary_id_type> boundary_ids;
    input_boundary_info.boundary_ids(node, boundary_ids);

    if (!boundary_ids.empty())
      primal_node_boundary_ids[roundedPointKey(*node)].push_back({*node, boundary_ids});
  }

  const auto addPrimalNodeBoundaryIDs = [&](Node * const dual_node, const Point & point)
  {
    const auto boundary_ids_it = primal_node_boundary_ids.find(roundedPointKey(point));

    if (boundary_ids_it == primal_node_boundary_ids.end())
      return;

    for (const auto & candidate : boundary_ids_it->second)
      if (MooseUtils::absoluteFuzzyEqual(candidate.point, point, dual_node_tol))
        dual_boundary_info.add_node(dual_node, candidate.boundary_ids);
  };

  const auto getDualNode = [&](const Point & point)
  {
    auto & candidate_nodes = dual_nodes_by_key[roundedPointKey(point)];

    for (auto * const node : candidate_nodes)
      if (MooseUtils::absoluteFuzzyEqual(*node, point, dual_node_tol))
        return node;

    Node * const node = dualMesh->add_point(point);
    candidate_nodes.push_back(node);
    addPrimalNodeBoundaryIDs(node, point);
    return node;
  };
  const auto copyPreservedPrimalElements = [&]()
  {
    if (preserve_primal_subdomain_ids.empty())
      return;

    std::map<dof_id_type, Node *> copied_nodes;

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
        dual_boundary_info.add_side(target_elem, target_side, ids_to_copy);
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
          primal_elem->set_node(n, polygon_nodes[n]);

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
        primal_elem->set_node(n, getPreservedPrimalNode(elem->node_id(n), elem->point(n)));

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
      if (pointInPolygon2D(boundary_segment.first, dual_points))
        addUniquePoint(clipped_points, boundary_segment.first, length_tol);

      if (pointInPolygon2D(boundary_segment.second, dual_points))
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
        if (pointOnSegment2D(unique_clipped_points[point_id], segment_start, segment_end))
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
      connectSegmentPoints(boundary_segment.first,
                           boundary_segment.second,
                           [&](const Point & midpoint)
                           { return pointInPolygon2D(midpoint, dual_points); });

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
      if (pointOnSegment2D(point, boundary_segment.first, boundary_segment.second))
        return true;

    return false;
  };
  // Concave element indexing helper, used in our later decomposition of concave elems.
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

  // Match boundary IDs after the final polygon decomposition so barycentric and Voronoi duals
  // share the same preservation path.
  const auto addPrimalSideBoundaryIDs = [&](Elem * const dual_elem,
                                            const BoundaryRegion2D * const boundary_region,
                                            const dof_id_type source_node_id)
  {
    if (boundary_region == nullptr)
      return;

    const auto segment_ids_it = boundary_region->boundary_node_to_segments.find(source_node_id);

    if (segment_ids_it == boundary_region->boundary_node_to_segments.end())
      return;

    std::set<boundary_id_type> candidate_boundary_ids;
    std::vector<boundary_id_type> source_node_boundary_ids;
    input_boundary_info.boundary_ids(input_mesh->node_ptr(source_node_id),
                                     source_node_boundary_ids);
    std::sort(source_node_boundary_ids.begin(), source_node_boundary_ids.end());

    for (const auto segment_id : segment_ids_it->second)
    {
      const auto & segment = boundary_region->preserved_boundary_segments[segment_id];
      candidate_boundary_ids.insert(segment.boundary_ids.begin(), segment.boundary_ids.end());
    }

    for (const auto side : dual_elem->side_index_range())
    {
      auto side_elem = dual_elem->build_side_ptr(side);

      if (side_elem->n_vertices() != 2)
        continue;

      const std::array<Point, 3> samples = {side_elem->point(0),
                                            0.5 * (side_elem->point(0) + side_elem->point(1)),
                                            side_elem->point(1)};
      std::vector<boundary_id_type> matched_boundary_ids;

      for (const auto boundary_id : candidate_boundary_ids)
      {
        bool all_samples_on_boundary = true;

        for (const auto & sample : samples)
        {
          bool sample_on_boundary = false;

          for (const auto segment_id : segment_ids_it->second)
          {
            const auto & segment = boundary_region->preserved_boundary_segments[segment_id];

            if (std::find(segment.boundary_ids.begin(), segment.boundary_ids.end(), boundary_id) !=
                    segment.boundary_ids.end() &&
                pointOnSegment2D(sample, segment.p0, segment.p1))
            {
              sample_on_boundary = true;
              break;
            }
          }

          if (!sample_on_boundary)
          {
            all_samples_on_boundary = false;
            break;
          }
        }

        if (all_samples_on_boundary)
          matched_boundary_ids.push_back(boundary_id);
      }

      if (!matched_boundary_ids.empty())
      {
        dual_boundary_info.add_side(
            dual_elem, cast_int<unsigned short>(side), matched_boundary_ids);

        std::vector<boundary_id_type> nodeset_ids;
        std::set_intersection(matched_boundary_ids.begin(),
                              matched_boundary_ids.end(),
                              source_node_boundary_ids.begin(),
                              source_node_boundary_ids.end(),
                              std::back_inserter(nodeset_ids));

        if (!nodeset_ids.empty())
          for (const auto n : dual_elem->nodes_on_side(side))
            dual_boundary_info.add_node(dual_elem->node_ptr(n), nodeset_ids);
      }
    }
  };

  const auto addDualElement = [&](const std::vector<Point> & points,
                                  const SubdomainID output_subdomain_id,
                                  const BoundaryRegion2D * const boundary_region,
                                  const dof_id_type source_node_id)
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
      Elem * const added_elem = dualMesh->add_elem(std::move(dual_elem));
      addPrimalSideBoundaryIDs(added_elem, boundary_region, source_node_id);
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
        if (fan_points.size() >= 3)
          addDualElement(fan_points, source_subdomain_id, boundary_region, source_node_id);

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
            addDualElement(triangle_points, source_subdomain_id, boundary_region, source_node_id);
        }
      }

      continue;
    }

    addDualElement(dual_points, source_subdomain_id, boundary_region, source_node_id);
  }

  dualMesh->unset_is_prepared();

  return dynamic_pointer_cast<MeshBase>(dualMesh);
}

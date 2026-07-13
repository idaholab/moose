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
#include <exception>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
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

// Triangulate a side-face part with the selected primal diagonal when one is available. NetGen and
// point-in-polyhedron checks both consume these triangles, so diagonal consistency controls whether
// adjacent dual cells share the same surface.
static std::vector<std::vector<Point>>
sideFacePartTriangles3D(const SideFacePart3D & side_face_part, const Real length_tol)
{
  const auto & node_ids = side_face_part.node_ids;
  const auto & points = side_face_part.points;
  std::vector<std::vector<Point>> triangles;

  if (points.size() < 3)
    return triangles;

  // Adds nondegenerate (usually, nonzero-area) triangles
  const auto addTriangle = [&](const std::array<std::size_t, 3> & point_indices)
  {
    std::vector<Point> triangle = {
        points[point_indices[0]], points[point_indices[1]], points[point_indices[2]]};

    if (hasNonzeroArea3D(triangle, length_tol * length_tol))
      triangles.push_back(std::move(triangle));
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

    return triangles;
  }

  for (const auto n : make_range(std::size_t(1), points.size() - 1))
    addTriangle({{0, n, n + 1}});

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

static void
appendDebugPoint3D(std::ostream & os, const Point & point)
{
  os << "(" << std::setprecision(17) << point(0) << ", " << point(1) << ", " << point(2) << ")";
}

static void
appendDebugDualPolyhedronPython3D(std::ostream & os,
                                  const std::vector<std::vector<Point>> & side_points,
                                  const Real tol)
{
  std::vector<Point> dual_nodes;
  std::vector<std::vector<std::size_t>> dual_faces;
  dual_faces.reserve(side_points.size());

  const auto nodeIndex = [&](const Point & point)
  {
    for (const auto i : index_range(dual_nodes))
      if (samePoint3D(dual_nodes[i], point, tol))
        return i;

    dual_nodes.push_back(point);
    return dual_nodes.size() - 1;
  };

  for (const auto & side : side_points)
  {
    std::vector<std::size_t> face;
    face.reserve(side.size());

    for (const auto & point : side)
      face.push_back(nodeIndex(point));

    dual_faces.push_back(std::move(face));
  }

  os << "dual_nodes = [\n";

  for (const auto & point : dual_nodes)
  {
    os << "    ";
    appendDebugPoint3D(os, point);
    os << ",\n";
  }

  os << "]\n\n";
  os << "dual_faces = [\n";

  for (const auto & face : dual_faces)
  {
    os << "    [";

    for (const auto i : index_range(face))
    {
      if (i)
        os << ", ";

      os << face[i];
    }

    os << "],\n";
  }

  os << "]\n\n";
  os << "dual_face_points = [[dual_nodes[i] for i in face] for face in dual_faces]\n";
}

static void
appendDebugPrimalElementsPython3D(std::ostream & os, const std::vector<const Elem *> & elems)
{
  std::vector<Point> primal_nodes;
  std::vector<std::vector<std::size_t>> primal_elements;
  std::vector<std::vector<std::size_t>> primal_faces;
  std::vector<dof_id_type> primal_element_ids;
  std::vector<SubdomainID> primal_element_subdomains;
  std::vector<std::size_t> primal_face_elements;
  std::map<dof_id_type, std::size_t> primal_node_indices;

  primal_elements.reserve(elems.size());
  primal_element_ids.reserve(elems.size());
  primal_element_subdomains.reserve(elems.size());

  const auto nodeIndex = [&](const Elem & elem, const unsigned int node_index)
  {
    const dof_id_type node_id = elem.node_id(node_index);
    const auto inserted = primal_node_indices.emplace(node_id, primal_node_indices.size());

    if (inserted.second)
      primal_nodes.push_back(elem.point(node_index));

    return inserted.first->second;
  };

  for (const auto * const elem : elems)
  {
    if (!elem)
      continue;

    const std::size_t primal_element_index = primal_elements.size();
    std::vector<std::size_t> primal_element;
    primal_element.reserve(elem->n_vertices());

    for (const auto node_index : make_range(elem->n_vertices()))
      primal_element.push_back(nodeIndex(*elem, node_index));

    for (const auto side : elem->side_index_range())
    {
      auto side_elem = elem->build_side_ptr(side);
      std::vector<std::size_t> primal_face;
      primal_face.reserve(side_elem->n_vertices());

      for (const auto node_index : make_range(side_elem->n_vertices()))
      {
        const auto primal_node_it = primal_node_indices.find(side_elem->node_id(node_index));

        if (primal_node_it != primal_node_indices.end())
          primal_face.push_back(primal_node_it->second);
      }

      if (primal_face.size() >= 3)
      {
        primal_faces.push_back(std::move(primal_face));
        primal_face_elements.push_back(primal_element_index);
      }
    }

    primal_elements.push_back(std::move(primal_element));
    primal_element_ids.push_back(elem->id());
    primal_element_subdomains.push_back(elem->subdomain_id());
  }

  os << "\nprimal_nodes = [\n";

  for (const auto & point : primal_nodes)
  {
    os << "    ";
    appendDebugPoint3D(os, point);
    os << ",\n";
  }

  os << "]\n\n";
  os << "primal_elements = [\n";

  for (const auto & elem : primal_elements)
  {
    os << "    [";

    for (const auto i : index_range(elem))
    {
      if (i)
        os << ", ";

      os << elem[i];
    }

    os << "],\n";
  }

  os << "]\n\n";
  os << "primal_faces = [\n";

  for (const auto & face : primal_faces)
  {
    os << "    [";

    for (const auto i : index_range(face))
    {
      if (i)
        os << ", ";

      os << face[i];
    }

    os << "],\n";
  }

  os << "]\n\n";
  os << "primal_element_ids = [";

  for (const auto i : index_range(primal_element_ids))
  {
    if (i)
      os << ", ";

    os << primal_element_ids[i];
  }

  os << "]\n\n";
  os << "primal_element_subdomains = [";

  for (const auto i : index_range(primal_element_subdomains))
  {
    if (i)
      os << ", ";

    os << primal_element_subdomains[i];
  }

  os << "]\n\n";
  os << "primal_face_elements = [";

  for (const auto i : index_range(primal_face_elements))
  {
    if (i)
      os << ", ";

    os << primal_face_elements[i];
  }

  os << "]\n\n";
  os << "primal_element_points = [[primal_nodes[i] for i in elem] for elem in "
        "primal_elements]\n";
  os << "primal_face_points = [[primal_nodes[i] for i in face] for face in primal_faces]\n";
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

// Split needs to add additional nodes along edges, and additional faces (child faces) in lieu of
// the faces we are splitting. These nodes are also added to their neighboring elems to maintain
// conformality
static void
insertSplitPointsOnSideEdges3D(std::vector<std::vector<Point>> & side_points,
                               const std::vector<SplitEdgePoint3D> & split_edge_points,
                               const std::vector<SplitFaceReplacement3D> & face_replacements,
                               const Real tol = 1e-12)
{
  if (split_edge_points.empty() && face_replacements.empty())
    return;

  using RoundedPointKey3D = std::array<long long, 3>;
  using RoundedSegmentKey3D = std::pair<RoundedPointKey3D, RoundedPointKey3D>;

  const Real key_tol = std::max(tol, Real(1e-12));

  const auto roundedPointKey = [&](const Point & point)
  {
    return RoundedPointKey3D{{static_cast<long long>(std::llround(point(0) / key_tol)),
                              static_cast<long long>(std::llround(point(1) / key_tol)),
                              static_cast<long long>(std::llround(point(2) / key_tol))}};
  };

  const auto roundedSegmentKey = [&](const Point & point0, const Point & point1)
  {
    auto key0 = roundedPointKey(point0);
    auto key1 = roundedPointKey(point1);

    if (key1 < key0)
      std::swap(key0, key1);

    return RoundedSegmentKey3D{key0, key1};
  };

  std::map<RoundedPointKey3D, std::vector<const SplitEdgePoint3D *>> split_edge_point_index;
  std::map<RoundedPointKey3D, std::vector<const SplitFaceReplacement3D *>> face_replacement_index;
  std::map<RoundedSegmentKey3D, std::vector<const SplitEdgePoint3D *>> split_edge_segment_index;

  const auto addSplitEdgePointCandidate = [](std::vector<const SplitEdgePoint3D *> & candidates,
                                             const SplitEdgePoint3D * split_edge_point)
  {
    for (const auto * const candidate : candidates)
      if (candidate == split_edge_point)
        return;

    candidates.push_back(split_edge_point);
  };

  // Makes sure replacement faces (for split/polycut) do not have duplicate pointers
  const auto addFaceReplacementCandidate =
      [](std::vector<const SplitFaceReplacement3D *> & candidates,
         const SplitFaceReplacement3D * face_replacement)
  {
    for (const auto * const candidate : candidates)
      if (candidate == face_replacement)
        return;

    candidates.push_back(face_replacement);
  };

  for (const auto & split_edge_point : split_edge_points)
    addSplitEdgePointCandidate(split_edge_segment_index[roundedSegmentKey(
                                   split_edge_point.edge_point0, split_edge_point.edge_point1)],
                               &split_edge_point);

  for (const auto & split_edge_segment : split_edge_segment_index)
    for (const auto * const split_edge_point : split_edge_segment.second)
    {
      addSplitEdgePointCandidate(
          split_edge_point_index[roundedPointKey(split_edge_point->edge_point0)], split_edge_point);
      addSplitEdgePointCandidate(
          split_edge_point_index[roundedPointKey(split_edge_point->edge_point1)], split_edge_point);

      for (const auto * const source_edge_split_point : split_edge_segment.second)
        addSplitEdgePointCandidate(
            split_edge_point_index[roundedPointKey(source_edge_split_point->split_point)],
            split_edge_point);
    }

  for (const auto & face_replacement : face_replacements)
    for (const auto & point : face_replacement.original_face_points)
      addFaceReplacementCandidate(face_replacement_index[roundedPointKey(point)],
                                  &face_replacement);

  // Gathers split-edge candidates
  const auto appendSplitEdgePointCandidates =
      [&](const Point & point, std::vector<const SplitEdgePoint3D *> & candidates)
  {
    const auto point_key = roundedPointKey(point);

    for (const auto dx : {-1, 0, 1})
      for (const auto dy : {-1, 0, 1})
        for (const auto dz : {-1, 0, 1})
        {
          const RoundedPointKey3D neighbor_key{
              {point_key[0] + dx, point_key[1] + dy, point_key[2] + dz}};
          const auto index_it = split_edge_point_index.find(neighbor_key);

          if (index_it == split_edge_point_index.end())
            continue;

          for (const auto * const split_edge_point : index_it->second)
            addSplitEdgePointCandidate(candidates, split_edge_point);
        }
  };

  const auto appendFaceReplacementCandidates =
      [&](const Point & point, std::vector<const SplitFaceReplacement3D *> & candidates)
  {
    const auto point_key = roundedPointKey(point);

    for (const auto dx : {-1, 0, 1})
      for (const auto dy : {-1, 0, 1})
        for (const auto dz : {-1, 0, 1})
        {
          const RoundedPointKey3D neighbor_key{
              {point_key[0] + dx, point_key[1] + dy, point_key[2] + dz}};
          const auto index_it = face_replacement_index.find(neighbor_key);

          if (index_it == face_replacement_index.end())
            continue;

          for (const auto * const face_replacement : index_it->second)
            addFaceReplacementCandidate(candidates, face_replacement);
        }
  };

  std::vector<std::vector<Point>> split_side_points;
  split_side_points.reserve(side_points.size());

  for (const auto & side : side_points)
  {
    bool replaced_side = false;

    if (side.size() == 4 && !face_replacements.empty())
    {
      std::vector<const SplitFaceReplacement3D *> candidate_face_replacements;

      for (const auto & point : side)
        appendFaceReplacementCandidates(point, candidate_face_replacements);

      for (const auto * const face_replacement : candidate_face_replacements)
        if (samePointSet3D(side, face_replacement->original_face_points, tol))
        {
          const Point side_normal = faceNormal3D(side, tol);

          addSidePoints3D(
              split_side_points,
              orientedSplitFacePart3D(face_replacement->child0_face_points, side_normal, tol),
              tol);
          addSidePoints3D(
              split_side_points,
              orientedSplitFacePart3D(face_replacement->child1_face_points, side_normal, tol),
              tol);
          replaced_side = true;
          break;
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
      std::vector<const SplitEdgePoint3D *> candidate_split_edge_points;

      appendSplitEdgePointCandidates(point0, candidate_split_edge_points);
      appendSplitEdgePointCandidates(point1, candidate_split_edge_points);

      for (const auto * const split_edge_point : candidate_split_edge_points)
      {
        if (samePoint3D(split_edge_point->split_point, point0, tol) ||
            samePoint3D(split_edge_point->split_point, point1, tol) ||
            !pointOnSegment3D(split_edge_point->split_point, point0, point1, tol))
          continue;

        const Real parameter = ((split_edge_point->split_point - point0) * edge) / edge_length_sq;
        bool already_added = false;

        for (const auto & edge_split_point : edge_split_points)
          if (samePoint3D(edge_split_point.second, split_edge_point->split_point, tol))
          {
            already_added = true;
            break;
          }

        if (!already_added)
          edge_split_points.push_back({parameter, split_edge_point->split_point});
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

  for (const auto i : index_range(side))
    if (!is_face_side_point[i])
    {
      ++n_body_points;

      if (!is_face_side_point[(i + side.size() - 1) % side.size()] ||
          !is_face_side_point[(i + 1) % side.size()])
        return false;
    }

  const std::size_t n_face_points = side.size() - n_body_points;

  if (n_body_points == 0 || n_face_points < 2)
    return false;

  matched = true;
  Real face_scale = 0.0;

  for (const auto i : index_range(side))
    for (const auto j : make_range(i + 1, side.size()))
      face_scale = std::max(face_scale, (side[j] - side[i]).norm());

  if (face_scale <= tol)
    return false;

  const Real area_tol = tol * face_scale;
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
      if (!valid_segment(chord_points[chord_i], chord_points[chord_i + 1]))
        return false;

      std::vector<Point> triangle = {chord_points[chord_i], side[i], chord_points[chord_i + 1]};

      if (!hasNonzeroArea3D(triangle, area_tol))
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

        if ((offset > 1 && !valid_segment(face_points[anchor], face_points[point1])) ||
            (offset + 1 < face_points.size() - 1 &&
             !valid_segment(face_points[anchor], face_points[point2])) ||
            (offset > 1 && segmentHasEmbeddedPoint(point1)) ||
            (offset + 1 < face_points.size() - 1 && segmentHasEmbeddedPoint(point2)))
        {
          valid_inner_fan = false;
          break;
        }

        std::vector<Point> triangle = {
            face_points[anchor], face_points[point1], face_points[point2]};

        if (!hasNonzeroArea3D(triangle, area_tol))
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

  const Real area_tol = tol * tol;
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

      if (!valid_segment(side[previous_index], side[next_index]))
        continue;

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
                                const Real tol = 1e-12,
                                std::string * const failure_reason = nullptr)
{
  if (failure_reason)
    failure_reason->clear();

  const auto fail = [&](const std::string & reason)
  {
    if (failure_reason)
      *failure_reason = reason;

    return false;
  };

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

  for (const auto triangle_i : index_range(surface_triangles))
  {
    const auto & triangle = surface_triangles[triangle_i];

    if (triangle.size() != 3 || !hasNonzeroArea3D(triangle, tol * tol))
    {
      std::ostringstream oss;
      oss << "surface triangle " << triangle_i << " is degenerate";
      return fail(oss.str());
    }

    for (const auto edge_i : make_range(std::size_t(3)))
    {
      const Point & edge_point0 = triangle[edge_i];
      const Point & edge_point1 = triangle[(edge_i + 1) % triangle.size()];

      for (const auto other_triangle_i : index_range(surface_triangles))
      {
        const auto & other_triangle = surface_triangles[other_triangle_i];

        for (const auto point_i : make_range(std::size_t(3)))
        {
          const Point & point = other_triangle[point_i];

          if (samePoint3D(point, edge_point0, tol) || samePoint3D(point, edge_point1, tol))
            continue;

          if (pointOnSegment3D(point, edge_point0, edge_point1, tol))
          {
            std::ostringstream oss;
            oss << "surface triangle " << other_triangle_i << " point " << point_i
                << " lies inside surface triangle " << triangle_i << " edge " << edge_i;
            return fail(oss.str());
          }
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

      for (const auto point_i : make_range(std::size_t(3)))
      {
        const Point & point = triangle0[point_i];

        if (!triangleContainsPoint(triangle1, point) &&
            pointOnTriangle3D(point, triangle1[0], triangle1[1], triangle1[2], tol))
        {
          std::ostringstream oss;
          oss << "surface triangle " << triangle0_i << " point " << point_i
              << " lies on surface triangle " << triangle1_i;
          return fail(oss.str());
        }
      }

      for (const auto point_i : make_range(std::size_t(3)))
      {
        const Point & point = triangle1[point_i];

        if (!triangleContainsPoint(triangle0, point) &&
            pointOnTriangle3D(point, triangle0[0], triangle0[1], triangle0[2], tol))
        {
          std::ostringstream oss;
          oss << "surface triangle " << triangle1_i << " point " << point_i
              << " lies on surface triangle " << triangle0_i;
          return fail(oss.str());
        }
      }

      for (const auto edge_i : make_range(std::size_t(3)))
      {
        Point intersection;

        if (segmentIntersectsTriangle(triangle0[edge_i],
                                      triangle0[(edge_i + 1) % triangle0.size()],
                                      triangle1,
                                      intersection) &&
            !intersectionIsOnSharedFeature(intersection, shared_points))
        {
          std::ostringstream oss;
          oss << "surface triangle " << triangle0_i << " edge " << edge_i
              << " intersects surface triangle " << triangle1_i << " at ";
          appendDebugPoint3D(oss, intersection);
          return fail(oss.str());
        }

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
            {
              std::ostringstream oss;
              oss << "surface triangle " << triangle0_i << " edge " << edge_i
                  << " crosses coplanar surface triangle " << triangle1_i << " edge "
                  << other_edge_i;
              return fail(oss.str());
            }
          }

        if (segmentIntersectsTriangle(triangle1[edge_i],
                                      triangle1[(edge_i + 1) % triangle1.size()],
                                      triangle0,
                                      intersection) &&
            !intersectionIsOnSharedFeature(intersection, shared_points))
        {
          std::ostringstream oss;
          oss << "surface triangle " << triangle1_i << " edge " << edge_i
              << " intersects surface triangle " << triangle0_i << " at ";
          appendDebugPoint3D(oss, intersection);
          return fail(oss.str());
        }
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
  auto dualMesh = buildReplicatedMesh(3);
  const auto preserve_primal_subdomain_ids = preservedPrimalSubdomainIDs(*input_mesh);

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
        addDualPointSource3D(dual_point_sources,
                             face_centroid,
                             side_face_part.has_selected_diagonal
                                 ? DualPointSource3D::face_diagonal_midpoint
                                 : DualPointSource3D::face_centroid);

        if (!side_is_preserved)
          continue;

        Point normal = faceNormal3D(side_face_points);

        if (normal.norm() <= 1e-12)
          continue;

        // Make sure face normals are oriented properly
        if (normal * (elem_centroid - face_centroid) > 0.0)
          normal = -1.0 * normal;

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

  const auto getDualNode = [&](const Point & point)
  {
    auto & candidate_nodes = dual_nodes_by_key[roundedPointKey(point)];

    for (auto * const node : candidate_nodes)
      if (MooseUtils::absoluteFuzzyEqual(*node, point, dual_node_tol))
        return std::make_pair(node, false);

    Node * const node = dualMesh->add_point(point);
    candidate_nodes.push_back(node);
    return std::make_pair(node, true);
  };

  // If we want to preserve subdomains (not dualize them) we copy them over
  const auto copyPreservedPrimalElements = [&]()
  {
    if (preserve_primal_subdomain_ids.empty())
      return;

    const BoundaryInfo & input_boundary_info = input_mesh->get_boundary_info();
    BoundaryInfo & boundary_info = dualMesh->get_boundary_info();
    std::map<dof_id_type, Node *> copied_nodes;

    const auto & input_sideset_map = input_boundary_info.get_sideset_name_map();
    const auto & input_nodeset_map = input_boundary_info.get_nodeset_name_map();

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

    const auto getPreservedPrimalNode = [&](const dof_id_type source_node_id, const Point & point)
    {
      const auto copied_node_it = copied_nodes.find(source_node_id);

      if (copied_node_it != copied_nodes.end())
        return copied_node_it->second;

      Node * const node = getDualNode(point).first;
      copied_nodes[source_node_id] = node;
      copyNodeBoundaryIDs(node, source_node_id);
      return node;
    };

    for (const auto & elem : input_mesh->element_ptr_range())
    {
      if (dualizeElem(*elem))
        continue;

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

  std::map<std::pair<SubdomainID, PointKey3D>, bool> primal_boundary_point_cache;
  std::map<std::pair<SubdomainID, SegmentKey3D>, bool> primal_boundary_segment_cache;

  struct SplitDualCellSidePoints3D
  {
    std::vector<std::vector<Point>> direct_netgen_side_points;
    std::vector<std::vector<Point>> polycut_side_points;
    SubdomainID output_subdomain_id = Elem::invalid_subdomain_id;
    bool has_concave_edge = false;
    PolyCutEdge3D concave_edge;
    bool force_tetrahedralize = false;
    bool has_split_plan = false;
    SplitCutFaceCandidate3D split_plan;
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

  /* C0Polyhedron throws mooseErrors when we try to build ill-behaved polyhedra directly. So, to be
   * able to build concave polyhedra, we need to run our own tests that emulate C0Polyhedron's
   * requirements, determine what's wrong, and then act accordingly. This is crucial for the
   * concave_treatment decision tree*/
  const auto canBuildPolyhedron = [&](MeshBase & mesh,
                                      const std::vector<std::vector<Point>> & side_points) -> bool
  {
    if (side_points.size() < 4)
      return false;

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    std::vector<std::vector<Point>> c0_side_points;

    if (!c0PolyhedronSidePoints3D(side_points, c0_side_points, length_tol, &dual_point_sources))
      return false;

    if (!polyhedronSurfaceCanBePassedToC0(c0_side_points, length_tol))
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
      auto polygon = std::make_shared<libMesh::C0Polygon>(side.size());

      for (const auto i : index_range(side))
        polygon->set_node(i, getLocalNode(side[i]));

      sides.push_back(polygon);
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
                                 const SubdomainID output_subdomain_id) -> bool
  {
    if (side_points.size() < 4)
      return false;

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    std::vector<std::vector<Point>> c0_side_points;
    std::vector<std::shared_ptr<libMesh::Polygon>> sides;

    if (!c0PolyhedronSidePoints3D(side_points, c0_side_points, length_tol, &dual_point_sources))
      return false;

    const auto getOrCreateDualNode = [&](const Point & point) { return getDualNode(point).first; };

    sides.reserve(c0_side_points.size());

    for (const auto & side : c0_side_points)
    {
      auto polygon = std::make_shared<libMesh::C0Polygon>(side.size());

      for (const auto i : index_range(side))
        polygon->set_node(i, getOrCreateDualNode(side[i]));

      sides.push_back(polygon);
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
    dualMesh->add_elem(std::move(dual_elem));

    return true;
  };

  // Polyhedra that are bad enough need to be triangulated and tetrahedralized via NetGen
  const auto addTetrahedralizedPolyhedron =
      [&](const std::vector<std::vector<Point>> & side_points,
          const SubdomainID output_subdomain_id,
          std::string & failure_reason,
          std::vector<std::vector<Point>> & failed_surface_triangles) -> std::size_t
  {
    failure_reason.clear();
    failed_surface_triangles.clear();

    const auto fail = [&](const std::string & reason)
    {
      failure_reason = reason;
      return std::size_t(0);
    };

    if (side_points.size() < 4)
      return fail("fewer than four surface sides were provided");

    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    const Real volume_tol = length_tol * length_tol * length_tol;
    std::vector<std::vector<Point>> surface_triangles;

    // Rejects surface diagonals (on nonplanar quads) that violate the primal boundary geometry
    const auto validSurfaceSegment = [&](const Point & point0, const Point & point1)
    { return segmentInsidePrimalBoundary(output_subdomain_id, point0, point1); };

    if (!surfaceTriangles3D(
            side_points, surface_triangles, validSurfaceSegment, length_tol, &dual_point_sources))
      return fail("could not triangulate the surface sides");

    failed_surface_triangles = surface_triangles;

    if (!orientSurfaceTriangles3D(surface_triangles, length_tol))
      return fail("could not orient the surface triangles");

    failed_surface_triangles = surface_triangles;

    std::string conformity_failure_reason;

    if (!surfaceTrianglesAreConforming3D(surface_triangles, length_tol, &conformity_failure_reason))
      return fail("surface triangles are not conforming: " + conformity_failure_reason);

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
          tri->set_node(n) = getBoundaryNode(triangle[n]);

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
      catch (const std::exception & error)
      {
        return fail(std::string("NetGen threw while tetrahedralizing the surface: ") +
                    error.what());
      }
      catch (...)
      {
        return fail("NetGen threw while tetrahedralizing the surface");
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

        const Point tet_center =
            (tet_points[0] + tet_points[1] + tet_points[2] + tet_points[3]) / 4.0;

        if (!pointInsidePrimalBoundary(output_subdomain_id, tet_center))
        {
          std::ostringstream oss;
          oss << "NetGen generated a tetrahedron outside the primal boundary; centroid = ";
          appendDebugPoint3D(oss, tet_center);
          return fail(oss.str());
        }

        generated_tets.push_back(tet_points);
      }

      if (generated_tets.empty())
        return fail("NetGen generated no nondegenerate tetrahedra");

      for (const auto & tet_points : generated_tets)
      {
        auto tet = std::make_unique<Tet4>();
        tet->set_node(0) = getDualNode(tet_points[0]).first;
        tet->set_node(1) = getDualNode(tet_points[1]).first;

        if (tetVolume6(tet_points[0], tet_points[1], tet_points[2], tet_points[3]) > 0.0)
        {
          tet->set_node(2) = getDualNode(tet_points[2]).first;
          tet->set_node(3) = getDualNode(tet_points[3]).first;
        }
        else
        {
          tet->set_node(2) = getDualNode(tet_points[3]).first;
          tet->set_node(3) = getDualNode(tet_points[2]).first;
        }

        if (output_subdomain_id != Elem::invalid_subdomain_id)
          tet->subdomain_id() = output_subdomain_id;
        dualMesh->add_elem(std::move(tet));
      }

      return generated_tets.size();
#else
      return fail("NetGen support is not available in libMesh");
#endif
    };

    return addNetgenTetrahedralizedSurface();
  };

  const auto addPolyCutPolyhedra = [&](const std::vector<std::vector<Point>> & side_points,
                                       const SubdomainID output_subdomain_id) -> std::size_t
  {
    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;
    const auto polycut_candidates =
        polyCutSidePointCandidates3D(side_points, boundary_normal_dot_tol, length_tol);

    // PolyCut has a number of requirements for candidate faces, but sometimes mismatches can slip
    // through. We want to make sure the children are valid, and if all possible candidates are
    // invalid, we move on to the next concave_treatment for this element.
    for (const auto & polycut_result : polycut_candidates)
    {
      auto validation_mesh = buildReplicatedMesh(3);

      if (!canBuildPolyhedron(*validation_mesh, polycut_result.child0_side_points) ||
          !canBuildPolyhedron(*validation_mesh, polycut_result.child1_side_points))
        continue;

      if (!addPolyhedron(polycut_result.child0_side_points, output_subdomain_id) ||
          !addPolyhedron(polycut_result.child1_side_points, output_subdomain_id))
        mooseError("Could not add polycut 3D dual polyhedron children.");

      return 2;
    }

    return 0;
  };

  const auto addSplitPolyhedra = [&](const std::vector<std::vector<Point>> & side_points,
                                     const SubdomainID output_subdomain_id,
                                     const SplitCutFaceCandidate3D * split_plan =
                                         nullptr) -> std::size_t
  {
    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(side_points);
    const Real length_tol = tol * polyhedron_scale;

    const auto addSplitChildPolyhedra =
        [&](const std::vector<std::vector<Point>> & child0_side_points,
            const std::vector<std::vector<Point>> & child1_side_points)
    {
      if (child0_side_points.empty() || child1_side_points.empty())
        return false;

      if (!addPolyhedron(child0_side_points, output_subdomain_id) ||
          !addPolyhedron(child1_side_points, output_subdomain_id))
        mooseError("Could not add split 3D dual polyhedron children.");

      return true;
    };

    const auto addSplitCandidatePolyhedra = [&](const SplitCutFaceCandidate3D & split_candidate)
    {
      std::vector<std::vector<Point>> child0_side_points;
      std::vector<std::vector<Point>> child1_side_points;
      const bool child0_built = buildPolyCutChildSidePoints3D(
          side_points, split_candidate.cut_face, true, length_tol, child0_side_points);
      const bool child1_built = buildPolyCutChildSidePoints3D(
          side_points, split_candidate.cut_face, false, length_tol, child1_side_points);

      if (!child0_built || !child1_built)
        return false;

      auto validation_mesh = buildReplicatedMesh(3);

      if (!canBuildPolyhedron(*validation_mesh, child0_side_points) ||
          !canBuildPolyhedron(*validation_mesh, child1_side_points))
        return false;

      return addSplitChildPolyhedra(child0_side_points, child1_side_points);
    };

    if (split_plan)
    {
      if (!split_plan->child0_side_points.empty() && !split_plan->child1_side_points.empty())
        return addSplitChildPolyhedra(split_plan->child0_side_points,
                                      split_plan->child1_side_points)
                   ? 2
                   : 0;

      return addSplitCandidatePolyhedra(*split_plan) ? 2 : 0;
    }

    PolyCutEdge3D concave_edge;

    if (!findConcavePolyhedronEdge3D(
            side_points, boundary_normal_dot_tol, length_tol, concave_edge))
      return 0;

    const auto split_candidates = splitCutFaceCandidates3D(side_points, concave_edge, length_tol);

    for (const auto & split_candidate : split_candidates)
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

      if (!canBuildPolyhedron(*validation_mesh, child0_side_points) ||
          !canBuildPolyhedron(*validation_mesh, child1_side_points))
        continue;

      split_plan = split_candidate;
      split_plan.child0_side_points = std::move(child0_side_points);
      split_plan.child1_side_points = std::move(child1_side_points);
      return true;
    }

    return false;
  };

  const auto rejectedNonConvexPolyhedronMessage =
      [&](const std::vector<std::vector<Point>> & rejected_side_points,
          const std::vector<const Elem *> & primal_elems,
          const std::string & netgen_failure_reason,
          const std::vector<std::vector<Point>> & failed_netgen_surface_triangles)
  {
    const Real tol = std::max(_geometry_relative_tol, Real(1e-12));
    const Real polyhedron_scale = polyhedronScale3D(rejected_side_points);
    const Real length_tol = tol * polyhedron_scale;
    std::vector<std::vector<Point>> c0_side_points;
    std::ostringstream oss;
    oss << "Could not resolve rejected non-convex 3D dual polyhedron.\n\n";
    oss << "# Paste into test.py\n";

    if (!netgen_failure_reason.empty())
    {
      std::string one_line_failure_reason = netgen_failure_reason;

      for (auto & character : one_line_failure_reason)
        if (character == '\n' || character == '\r')
          character = ' ';

      oss << "# netgen_failure_reason = \"" << one_line_failure_reason << "\"\n";
    }

    if (!failed_netgen_surface_triangles.empty())
    {
      oss << "# dual_faces are the oriented surface triangles checked before NetGen\n";
      appendDebugDualPolyhedronPython3D(oss, failed_netgen_surface_triangles, length_tol);
    }
    else if (c0PolyhedronSidePoints3D(
                 rejected_side_points, c0_side_points, length_tol, &dual_point_sources))
    {
      oss << "# dual_faces are C0-prepared triangles\n";
      appendDebugDualPolyhedronPython3D(oss, c0_side_points, length_tol);
    }
    else
    {
      oss << "# dual_faces are raw rejected sides; C0 side preparation failed\n";
      appendDebugDualPolyhedronPython3D(oss, rejected_side_points, length_tol);
    }

    appendDebugPrimalElementsPython3D(oss, primal_elems);

    return oss.str();
  };

  // Determines what concave_treatment based on results
  const auto addPolyhedronOrTetrahedralize =
      [&](const std::vector<std::vector<Point>> & direct_netgen_side_points,
          const std::vector<std::vector<Point>> & polycut_side_points,
          const bool force_tetrahedralize,
          const bool searched_concave_edge,
          const bool has_concave_edge,
          const SubdomainID output_subdomain_id,
          const std::vector<const Elem *> & primal_elems,
          const SplitCutFaceCandidate3D * split_plan = nullptr)
  {
    const bool concave_edge_search_failed =
        searched_concave_edge && !has_concave_edge && !split_plan;
    const std::vector<std::vector<Point>> * rejected_side_points = &polycut_side_points;
    std::string netgen_failure_reason;
    std::vector<std::vector<Point>> failed_netgen_surface_triangles;

    if (!force_tetrahedralize && !has_concave_edge && !split_plan)
    {
      if (addPolyhedron(direct_netgen_side_points, output_subdomain_id))
        return;
    }

    if (use_split && !force_tetrahedralize && !has_concave_edge && !split_plan)
    {
      if (addPolyhedron(polycut_side_points, output_subdomain_id))
        return;
    }

    for (const auto & concave_treatment : _concave_treatment)
      if (concave_treatment == "split")
      {
        if (concave_edge_search_failed)
          continue;

        const std::size_t created_elements =
            addSplitPolyhedra(polycut_side_points, output_subdomain_id, split_plan);

        if (created_elements)
          return;
      }
      else if (concave_treatment == "polycut")
      {
        if (concave_edge_search_failed)
          continue;

        const std::size_t created_elements =
            addPolyCutPolyhedra(polycut_side_points, output_subdomain_id);

        if (created_elements)
          return;
      }
      else if (concave_treatment == "netgen")
      {
        const std::size_t netgen_elements =
            addTetrahedralizedPolyhedron(direct_netgen_side_points,
                                         output_subdomain_id,
                                         netgen_failure_reason,
                                         failed_netgen_surface_triangles);

        if (netgen_elements)
          return;

        rejected_side_points = &direct_netgen_side_points;
      }

    mooseError(rejectedNonConvexPolyhedronMessage(*rejected_side_points,
                                                  primal_elems,
                                                  netgen_failure_reason,
                                                  failed_netgen_surface_triangles));
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
    std::map<EdgeSubdomainKey, std::vector<Point>> midpoint_boundary_face_centroids;
    std::map<EdgeSubdomainKey, std::vector<Point>> boundary_edge_face_centroids;
    ConnectedFacePoints3D boundary_face_points;
    Point boundary_normal;

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
                                          const Point * next_midpoint)
    {
      if (normal.norm() <= primal_boundary_length_tol)
        return;

      const Point unit_normal = normal / normal.norm();
      const Real plane_constant = unit_normal * face_centroid;
      const Real normal_tol = 1e-8;
      std::size_t plane_index = boundary_plane_faces.size();

      for (const auto i : index_range(boundary_plane_faces))
      {
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

      addBoundaryPlaneEdgePoints(
          boundary_plane_faces[plane_index], previous_node_id, face_centroid, previous_midpoint);
      addBoundaryPlaneEdgePoints(
          boundary_plane_faces[plane_index], next_node_id, face_centroid, next_midpoint);
    };

    // If we construct aboundary face that is open at the primal source vertex, we should connect it
    // to the vertex
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

      const Point endpoint_delta = endpoints[1] - endpoints[0];
      const Point source_delta = source_point - endpoints[0];
      const Real endpoint_length_sq = endpoint_delta.norm_sq();
      const Real endpoint_length = std::sqrt(endpoint_length_sq);

      if (endpoint_length_sq > primal_boundary_length_tol * primal_boundary_length_tol &&
          libMesh::cross_norm(endpoint_delta, source_delta) <=
              primal_boundary_length_tol * endpoint_length)
      {
        const Real source_parameter = (source_delta * endpoint_delta) / endpoint_length_sq;
        const Real parameter_tol = primal_boundary_length_tol / endpoint_length;

        if (source_parameter >= -parameter_tol && source_parameter <= 1.0 + parameter_tol)
        {
          addConnectedFaceSegment3D(boundary_plane_face.face_points,
                                    endpoints[0],
                                    endpoints[1],
                                    primal_boundary_length_tol);
          return;
        }
      }

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

          // Route every dual face through the primal face's dual point. For an interior face, the
          // neighboring element contributes the other body-centroid-to-face-centroid segment.
          addConnectedFaceSegment3D(
              previous_edge_points, elem_centroid, face_centroid, primal_boundary_length_tol);
          addConnectedFaceSegment3D(
              next_edge_points, elem_centroid, face_centroid, primal_boundary_length_tol);

          if (side_is_preserved)
          {
            addConnectedFacePoint3D(
                boundary_face_points, face_centroid, primal_boundary_length_tol);

            const auto previous_boundary_edge_key =
                EdgeSubdomainKey{previous_edge_key, source_subdomain_key};
            const auto next_boundary_edge_key =
                EdgeSubdomainKey{next_edge_key, source_subdomain_key};
            addUniquePoint(boundary_edge_face_centroids[previous_boundary_edge_key],
                           face_centroid,
                           primal_boundary_length_tol);
            addUniquePoint(boundary_edge_face_centroids[next_boundary_edge_key],
                           face_centroid,
                           primal_boundary_length_tol);

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
                addUniquePoint(midpoint_boundary_face_centroids[previous_boundary_edge_key],
                               face_centroid,
                               primal_boundary_length_tol);
              }

              if (next_midpoint)
              {
                addConnectedFaceSegment3D(
                    next_edge_points, face_centroid, *next_midpoint, primal_boundary_length_tol);
                addUniquePoint(midpoint_boundary_face_centroids[next_boundary_edge_key],
                               face_centroid,
                               primal_boundary_length_tol);
              }
            }

            Point normal = faceNormal3D(current_side_points);

            if (normal.norm() > 1e-12)
            {
              if (normal * (elem_centroid - face_centroid) > 0.0)
                normal = -1.0 * normal;

              boundary_normal += normal / normal.norm();

              addBoundaryPlaneFace(normal,
                                   face_centroid,
                                   previous_node_id,
                                   next_node_id,
                                   previous_midpoint,
                                   next_midpoint);
            }
          }
        }
      }
    }

    // Keep the direct/netgen and polycut boundary face workflows separate. polycut needs whole
    // boundary-plane faces, while direct C0Polyhedron/netgen keep the legacy split faces.
    std::vector<std::vector<Point>> direct_netgen_side_points;
    std::vector<std::vector<Point>> polycut_side_points;
    std::vector<std::pair<Point, Point>> midpoint_split_boundary_faces;

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

      addSidePoints3D(direct_netgen_side_points, sorted_edge_points);
      addSidePoints3D(polycut_side_points, sorted_edge_points);
    }

    for (const auto & midpoint_face_centroids : midpoint_boundary_face_centroids)
    {
      const Point * const midpoint = boundaryEdgeMidpoint(midpoint_face_centroids.first);

      if (!midpoint)
        continue;

      for (const auto & face_centroid : midpoint_face_centroids.second)
        addSidePoints3D(direct_netgen_side_points,
                        {source_point, *midpoint, face_centroid},
                        primal_boundary_length_tol);

      for (const auto i : index_range(midpoint_face_centroids.second))
        for (const auto j : make_range(i + 1, midpoint_face_centroids.second.size()))
          midpoint_split_boundary_faces.push_back(
              {midpoint_face_centroids.second[i], midpoint_face_centroids.second[j]});
    }

    for (const auto & edge_face_centroids : boundary_edge_face_centroids)
      if (edge_face_centroids.second.size() == 2)
        addConnectedFaceSegment3D(boundary_face_points,
                                  edge_face_centroids.second[0],
                                  edge_face_centroids.second[1],
                                  primal_boundary_length_tol);

    if (boundary_face_points.points.size() >= 2)
    {
      const Point boundary_axis = boundary_normal.norm() > 1e-12 ? boundary_normal : source_point;
      const auto sorted_boundary_points = sortConnectedFacePoints3D(
          boundary_face_points, boundary_axis, primal_boundary_length_tol);

      // Have to avoid re-adding a boundary edge already split by midpoint handling
      const auto boundaryFaceWasSplit = [&](const Point & point0, const Point & point1)
      {
        for (const auto & split_boundary_face : midpoint_split_boundary_faces)
          if ((samePoint3D(point0, split_boundary_face.first, primal_boundary_length_tol) &&
               samePoint3D(point1, split_boundary_face.second, primal_boundary_length_tol)) ||
              (samePoint3D(point0, split_boundary_face.second, primal_boundary_length_tol) &&
               samePoint3D(point1, split_boundary_face.first, primal_boundary_length_tol)))
            return true;

        return false;
      };

      if (boundary_vertex_nodes.count(source_node_subdomain_key))
      {
        if (sorted_boundary_points.size() == 2)
        {
          if (!boundaryFaceWasSplit(sorted_boundary_points[0], sorted_boundary_points[1]))
            addSidePoints3D(direct_netgen_side_points,
                            {source_point, sorted_boundary_points[0], sorted_boundary_points[1]},
                            primal_boundary_length_tol);
        }
        else
          for (const auto i : index_range(sorted_boundary_points))
          {
            const Point & point0 = sorted_boundary_points[i];
            const Point & point1 = sorted_boundary_points[(i + 1) % sorted_boundary_points.size()];

            if (!boundaryFaceWasSplit(point0, point1))
              addSidePoints3D(direct_netgen_side_points,
                              {source_point, point0, point1},
                              primal_boundary_length_tol);
          }
      }
      else if (sorted_boundary_points.size() >= 3)
        addSidePoints3D(
            direct_netgen_side_points, sorted_boundary_points, primal_boundary_length_tol);
    }

    for (auto & boundary_plane_face : boundary_plane_faces)
    {
      for (const auto & boundary_edge_points : boundary_plane_face.edge_points)
      {
        if (boundary_edge_points.has_midpoint)
          for (const auto & face_centroid : boundary_edge_points.face_centroids)
            addConnectedFaceSegment3D(boundary_plane_face.face_points,
                                      face_centroid,
                                      boundary_edge_points.midpoint,
                                      primal_boundary_length_tol);
        else if (boundary_edge_points.face_centroids.size() == 2)
          addConnectedFaceSegment3D(boundary_plane_face.face_points,
                                    boundary_edge_points.face_centroids[0],
                                    boundary_edge_points.face_centroids[1],
                                    primal_boundary_length_tol);
        else
          for (const auto & face_centroid : boundary_edge_points.face_centroids)
            addConnectedFacePoint3D(
                boundary_plane_face.face_points, face_centroid, primal_boundary_length_tol);
      }

      closeBoundaryPlaneFaceAtSource(boundary_plane_face);

      addSidePoints3D(polycut_side_points,
                      sortConnectedFacePoints3D(boundary_plane_face.face_points,
                                                boundary_plane_face.normal,
                                                primal_boundary_length_tol),
                      primal_boundary_length_tol);
    }

    if (direct_netgen_side_points.size() < 4)
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
    const bool has_nonconvex_face_plane =
        !has_concave_edge && !boundary_plane_faces.empty() &&
        hasPointOutsidePolyhedronFacePlanes3D(polycut_side_points, concavity_length_tol);

    const bool force_tetrahedralize =
        (has_concave_boundary_normals && has_concave_edge) || has_nonconvex_face_plane;

    if (use_split)
      split_dual_cell_side_points.push_back({direct_netgen_side_points,
                                             polycut_side_points,
                                             source_subdomain_key,
                                             has_concave_edge,
                                             concave_edge,
                                             force_tetrahedralize,
                                             false,
                                             {},
                                             node_elems.second});
    else
      addPolyhedronOrTetrahedralize(direct_netgen_side_points,
                                    polycut_side_points,
                                    force_tetrahedralize,
                                    searched_concave_edge,
                                    has_concave_edge,
                                    source_subdomain_key,
                                    node_elems.second);
  }

  if (use_split)
  {
    std::vector<SplitEdgePoint3D> split_edge_points;
    std::vector<SplitFaceReplacement3D> split_face_replacements;

    for (auto & split_dual_cell_side_point : split_dual_cell_side_points)
    {
      if (!split_dual_cell_side_point.has_concave_edge)
        continue;

      if (findValidSplitPlan(split_dual_cell_side_point.polycut_side_points,
                             split_dual_cell_side_point.concave_edge,
                             split_dual_cell_side_point.split_plan))
      {
        split_dual_cell_side_point.has_split_plan = true;

        for (const auto & split_edge_point :
             split_dual_cell_side_point.split_plan.split_edge_points)
          addUniqueSplitEdgePoint3D(
              split_edge_points, split_edge_point, primal_boundary_length_tol);

        for (const auto & face_replacement :
             split_dual_cell_side_point.split_plan.split_face_replacements)
          addUniqueSplitFaceReplacement3D(
              split_face_replacements, face_replacement, primal_boundary_length_tol);
      }
    }

    for (auto & split_dual_cell_side_point : split_dual_cell_side_points)
    {
      insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.direct_netgen_side_points,
                                     split_edge_points,
                                     split_face_replacements,
                                     primal_boundary_length_tol);
      insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.polycut_side_points,
                                     split_edge_points,
                                     split_face_replacements,
                                     primal_boundary_length_tol);

      if (split_dual_cell_side_point.has_split_plan)
      {
        insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.split_plan.child0_side_points,
                                       split_edge_points,
                                       split_face_replacements,
                                       primal_boundary_length_tol);
        insertSplitPointsOnSideEdges3D(split_dual_cell_side_point.split_plan.child1_side_points,
                                       split_edge_points,
                                       split_face_replacements,
                                       primal_boundary_length_tol);
      }

      addPolyhedronOrTetrahedralize(split_dual_cell_side_point.direct_netgen_side_points,
                                    split_dual_cell_side_point.polycut_side_points,
                                    split_dual_cell_side_point.force_tetrahedralize,
                                    true,
                                    split_dual_cell_side_point.has_concave_edge,
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

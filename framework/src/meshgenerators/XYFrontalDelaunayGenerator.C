//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYFrontalDelaunayGenerator.h"

#include "GeometryUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/utility.h"

// C++ includes
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>

registerMooseObject("MooseApp", XYFrontalDelaunayGenerator);

// The mesh generator sources are compiled as one unity translation unit, which puts this file scope
// and that of every sibling generator together, so everything here carries the name of this one
namespace
{
/// Number of sides of a triangle
constexpr unsigned int n_frontal_tri_sides = 3;

/// @return The plain coordinates of a point of the triangulation as a mesh point
Point
frontalToPoint(const IncrementalDelaunay::Point2D & point)
{
  return Point(point.x, point.y, 0.0);
}

/// @return Whether one point comes before another in the plane, ordered on x and then on y
bool
frontalPointLess(const Point & first, const Point & second)
{
  return (first(0) != second(0)) ? first(0) < second(0) : first(1) < second(1);
}

/**
 * Puts a closed loop of the input boundary in the one form the seeding accepts: running
 * counter-clockwise and opening at its lowest point. The vertex ids the loop is seeded with number
 * the output mesh and break the ties of the advance, so both have to follow from the geometry of
 * the loop rather than from the direction it was walked in or the point it was opened at, neither
 * of which the extraction promises.
 * @param loop The corners of the loop, reordered in place
 */
void
frontalCanonicalizeLoop(std::vector<Point> & loop)
{
  if (geom_utils::signedArea2D(loop) < 0.0)
    std::reverse(loop.begin(), loop.end());

  std::rotate(
      loop.begin(), std::min_element(loop.begin(), loop.end(), frontalPointLess), loop.end());
}

/**
 * The circumradius of a triangle, infinite when its three vertices are collinear.
 *
 * The corners are put in a fixed order and the sides multiplied in a fixed order, so that the
 * result depends on the set of three corners and not on which of them the caller passed first.
 * Rounding otherwise makes two congruent triangles differ in the last bit, which is enough to
 * decide which of two front edges is advanced first and leaves the order of the advance following
 * the order the triangles happen to be stored in rather than the tie-break written for it.
 */
Real
frontalCircumradius(const Point & first, const Point & second, const Point & third)
{
  std::array<Point, 3> corners = {first, second, third};
  std::sort(corners.begin(), corners.end(), frontalPointLess);

  const Real twice_area = (corners[1](0) - corners[0](0)) * (corners[2](1) - corners[0](1)) -
                          (corners[1](1) - corners[0](1)) * (corners[2](0) - corners[0](0));
  if (twice_area == 0.0)
    return std::numeric_limits<Real>::max();

  std::array<Real, 3> sides = {(corners[1] - corners[0]).norm(),
                               (corners[2] - corners[1]).norm(),
                               (corners[0] - corners[2]).norm()};
  std::sort(sides.begin(), sides.end());

  return sides[0] * sides[1] * sides[2] / (2.0 * std::abs(twice_area));
}

/**
 * The point at LINF distance size from both ends of a front edge, on the side the front advances
 * to. The two squares of half width size about the ends meet in a box, and only two of that box's
 * corners lie on both squares; they straddle the edge, so the inward normal picks between them.
 * Placing the point on a corner rather than between them is what makes the new triangle a right
 * isosceles one, the shape that recombines into a good quadrilateral.
 * @param start The vertex at the start of the front edge
 * @param end The vertex at the end of the front edge
 * @param normal The unit normal of the edge pointing to the side the front advances to
 * @param size The target edge length
 * @param frame The frame the LINF norm is measured in
 * @return The point to place
 */
Point
frontalLinfCorner(const Point & start,
                  const Point & end,
                  const Point & normal,
                  const Real size,
                  const std::pair<Point, Point> & frame)
{
  const Point & u = frame.first;
  const Point & v = frame.second;

  const Real start_u = start * u;
  const Real start_v = start * v;
  const Real end_u = end * u;
  const Real end_v = end * v;

  // Neither square reaches the other end once the edge is longer than twice the target size in a
  // frame coordinate, so the target grows to whatever that edge needs, as it does for the L2 metric
  const Real reach =
      std::max({size, 0.5 * std::abs(end_u - start_u), 0.5 * std::abs(end_v - start_v)});

  const Real low_u = std::max(start_u, end_u) - reach;
  const Real high_u = std::min(start_u, end_u) + reach;
  const Real low_v = std::max(start_v, end_v) - reach;
  const Real high_v = std::min(start_v, end_v) + reach;

  // A corner sits at the target distance from the end that is larger in u and from the end that is
  // larger in v, so which pair of corners qualifies turns on whether that is the same end
  const bool same_end_larger = ((end_u >= start_u) == (end_v >= start_v));
  const Point first = low_u * u + (same_end_larger ? high_v : low_v) * v;
  const Point second = high_u * u + (same_end_larger ? low_v : high_v) * v;

  return ((first - start) * normal > (second - start) * normal) ? first : second;
}

/**
 * Marks the triangles that lie in the domain. The constrained segments are exactly the outer
 * boundary and the hole boundaries, so spreading from one triangle known to lie in the domain
 * across every edge that is not constrained reaches the domain and nothing else.
 * @param delaunay The triangulation
 * @param triangles The triangles of that triangulation
 * @param seed_start The vertex at the start of a boundary segment that has the domain on its left
 * @param seed_end The vertex at the end of that segment
 * @return Whether each triangle lies in the domain
 */
std::vector<bool>
frontalInsideTriangles(const IncrementalDelaunay & delaunay,
                       const std::vector<IncrementalDelaunay::Triangle> & triangles,
                       const std::size_t seed_start,
                       const std::size_t seed_end)
{
  // The triangles are counter-clockwise, so the one that holds the segment in the order that puts
  // the domain on its left is the one on the domain side of it
  std::size_t seed = IncrementalDelaunay::invalid_index;
  for (const auto t : index_range(triangles))
    for (const auto k : make_range(n_frontal_tri_sides))
      if (triangles[t].vertices[k] == seed_start &&
          triangles[t].vertices[(k + 1) % n_frontal_tri_sides] == seed_end)
        seed = t;

  if (seed == IncrementalDelaunay::invalid_index)
    mooseError("The segment from vertex ",
               seed_start,
               " to vertex ",
               seed_end,
               " of the outer boundary is not an edge of the triangulation, so the triangles that "
               "lie in the domain cannot be found. A point placed exactly on that segment divides "
               "it in two, which removes it.");

  std::vector<bool> inside(triangles.size(), false);
  inside[seed] = true;

  std::vector<std::size_t> pending{seed};

  while (!pending.empty())
  {
    const auto current = pending.back();
    pending.pop_back();

    const auto & triangle = triangles[current];
    for (const auto i : make_range(n_frontal_tri_sides))
    {
      const auto neighbor = triangle.neighbors[i];
      if (neighbor == IncrementalDelaunay::invalid_index || inside[neighbor])
        continue;
      if (delaunay.isConstrainedSegment(triangle.vertices[(i + 1) % n_frontal_tri_sides],
                                        triangle.vertices[(i + 2) % n_frontal_tri_sides]))
        continue;

      inside[neighbor] = true;
      pending.push_back(neighbor);
    }
  }

  return inside;
}
}

InputParameters
XYFrontalDelaunayGenerator::validParams()
{
  InputParameters params = SurfaceDelaunayGeneratorBase::validParams();
  params += SurfaceDelaunayGeneratorBase::boundaryAndHolesParams();

  MooseEnum metric("L2 LINF", "LINF");
  MooseEnum orientation("BOUNDARY CROSS_FIELD", "CROSS_FIELD");

  params.addParam<std::vector<Point>>(
      "interior_points",
      {},
      "Interior node locations. Any point outside the surface will not be meshed.");

  params.addParam<MooseEnum>("metric",
                             metric,
                             "The norm the target size is measured in when a point is placed ahead "
                             "of the front. 'L2' places points that make equilateral triangles. "
                             "'LINF' places points that make right isosceles triangles in the "
                             "local frame, the shape that recombines into good quadrilaterals.");
  params.addParam<MooseEnum>("orientation",
                             orientation,
                             "Where the local frame the 'LINF' metric measures in comes from. "
                             "'CROSS_FIELD' solves for a cross field over the domain. 'BOUNDARY' "
                             "takes the frame of the nearest boundary segment, which needs no "
                             "solve. This parameter has no effect when metric is 'L2'.");

  params.addParamNamesToGroup("interior_points", "Mandatory mesh interior nodes");
  params.addParamNamesToGroup("metric orientation", "Frontal advance");

  params.addClassDescription(
      "Triangulates meshes within boundaries defined by input meshes by advancing a front, which "
      "places points at a target size ahead of the triangles that are still too large.");

  return params;
}

XYFrontalDelaunayGenerator::XYFrontalDelaunayGenerator(const InputParameters & parameters)
  : SurfaceDelaunayGeneratorBase(parameters),
    _bdy_ptr(getMesh("boundary")),
    _hole_ptrs(getMeshes("holes")),
    _add_nodes_per_boundary_segment(getParam<unsigned int>("add_nodes_per_boundary_segment")),
    _refine_bdy(getParam<bool>("refine_boundary")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _refine_holes(getParam<std::vector<bool>>("refine_holes")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _interior_points(getParam<std::vector<Point>>("interior_points")),
    _metric(getParam<MooseEnum>("metric")),
    _orientation(getParam<MooseEnum>("orientation")),
    _background_mean_area(0.0),
    _boundary_cell(0.0),
    _grid_cell(0.0)
{
  checkBoundaryAndHolesParams(_hole_ptrs);
  checkInteriorPoints(_interior_points);

  // The frame the orientation selects only enters the LINF metric, so it does nothing under L2
  if (_metric == "L2" && isParamSetByUser("orientation"))
    paramError("orientation", "This parameter only applies to the 'LINF' metric.");
}

std::unique_ptr<MeshBase>
XYFrontalDelaunayGenerator::buildBackgroundMesh(
    const MeshBase & boundary_mesh,
    const std::vector<std::unique_ptr<MeshBase>> & holes,
    const MeshTriangulationUtils::XYDelaunayOptions & opts)
{
  MeshTriangulationUtils::XYDelaunayOptions background_opts = opts;

  // The background mesh only has to cover the domain, so it carries none of the naming and none of
  // the stitching the output mesh gets
  background_opts.stitch_holes.clear();
  background_opts.hole_boundaries.clear();
  background_opts.has_output_subdomain_name = false;
  background_opts.has_output_boundary = false;

  background_opts.desired_area = _background_area_factor * opts.desired_area;
  if (!opts.desired_area_func.empty())
    background_opts.desired_area_func =
        std::to_string(_background_area_factor) + "*(" + opts.desired_area_func + ")";

  std::vector<std::unique_ptr<MeshBase>> hole_clones;
  for (const auto & hole : holes)
    hole_clones.push_back(hole->clone());

  return MeshTriangulationUtils::triangulateWithDelaunay(
      *this, boundary_mesh.clone(), std::move(hole_clones), background_opts);
}

std::map<dof_id_type, Real>
XYFrontalDelaunayGenerator::boundaryTangentAngles(const MeshBase & mesh)
{
  std::map<dof_id_type, std::complex<Real>> directions;

  for (const auto & elem : mesh.element_ptr_range())
    for (const auto side : elem->side_index_range())
    {
      if (elem->neighbor_ptr(side))
        continue;

      const Node & start = elem->node_ref(side);
      const Node & end = elem->node_ref((side + 1) % elem->n_sides());

      // Accumulating exp(4 i theta) rather than theta averages the directions modulo pi / 2, the
      // symmetry a cross carries, so that the two sides of a right angle agree with each other
      const std::complex<Real> direction =
          std::polar(1.0, 4.0 * std::atan2(end(1) - start(1), end(0) - start(0)));
      directions[start.id()] += direction;
      directions[end.id()] += direction;
    }

  std::map<dof_id_type, Real> angles;
  for (const auto & [node_id, direction] : directions)
    angles[node_id] = std::arg(direction) / 4.0;

  return angles;
}

void
XYFrontalDelaunayGenerator::appendLoop(const std::vector<Point> & loop,
                                       const bool refine,
                                       const unsigned int extra_nodes,
                                       const boundary_id_type bcid,
                                       std::vector<Point> & points,
                                       std::vector<IncrementalDelaunay::Segment> & segments)
{
  const std::size_t loop_start = points.size();

  for (const auto i : index_range(loop))
  {
    const Point & start = loop[i];
    const Point & end = loop[(i + 1) % loop.size()];
    _boundary_segments.emplace_back(start, end);

    unsigned int pieces = extra_nodes + 1;
    if (refine)
    {
      // Splitting a segment the front would otherwise have to advance along is the only chance to
      // refine it, because the front never moves a constrained segment
      const Real size = targetSize(targetArea(0.5 * (start + end)));
      pieces = std::max(pieces, static_cast<unsigned int>(std::ceil((end - start).norm() / size)));
    }

    points.push_back(start);
    for (const auto piece : make_range(1u, pieces))
      points.push_back(start + (Real(piece) / pieces) * (end - start));
  }

  for (const auto vertex : make_range(loop_start, points.size()))
  {
    const std::size_t next = (vertex + 1 < points.size()) ? vertex + 1 : loop_start;
    segments.emplace_back(vertex, next);
    _segment_boundary_ids[IncrementalDelaunay::makeSegment(vertex, next)] = bcid;
  }
}

Real
XYFrontalDelaunayGenerator::targetArea(const Point & point) const
{
  if (_area_function)
  {
    const Real area = (*_area_function)(point);
    if (area <= 0.0)
      paramError("desired_area_func",
                 "The desired area must be positive everywhere in the meshed domain, but it is ",
                 area,
                 " at ",
                 point,
                 ".");
    return area;
  }

  if (_desired_area > 0.0)
    return _desired_area;

  // With no area limit of its own the advance targets the background triangulation, which the
  // automatic area function or the spacing of the boundary points sized
  const Elem * const background_elem = (*_background_locator)(point);

  return background_elem ? background_elem->volume() : _background_mean_area;
}

Real
XYFrontalDelaunayGenerator::targetSize(const Real area) const
{
  // The equilateral triangle of side h has area sqrt(3) h^2 / 4, the right isosceles triangle of
  // legs h has area h^2 / 2
  return (_metric == "L2") ? std::sqrt(4.0 * area / std::sqrt(3.0)) : std::sqrt(2.0 * area);
}

Real
XYFrontalDelaunayGenerator::targetCircumradius(const Real size) const
{
  // The equilateral triangle of side h has circumradius h / sqrt(3), the right isosceles triangle
  // of legs h has half of its hypotenuse
  return (_metric == "L2") ? size / std::sqrt(3.0) : size / std::sqrt(2.0);
}

std::pair<long, long>
XYFrontalDelaunayGenerator::gridKey(const Point & point, const Real cell) const
{
  return {static_cast<long>(std::floor(point(0) / cell)),
          static_cast<long>(std::floor(point(1) / cell))};
}

void
XYFrontalDelaunayGenerator::buildBoundarySegmentGrid()
{
  mooseAssert(!_boundary_segments.empty(),
              "The boundary was seeded before the grid over it is built.");

  Real min_x = std::numeric_limits<Real>::max();
  Real max_x = std::numeric_limits<Real>::lowest();
  Real min_y = min_x;
  Real max_y = max_x;
  for (const auto & [start, end] : _boundary_segments)
    for (const auto & corner : {start, end})
    {
      min_x = std::min(min_x, corner(0));
      max_x = std::max(max_x, corner(0));
      min_y = std::min(min_y, corner(1));
      max_y = std::max(max_y, corner(1));
    }

  // Buckets that hold about one segment each: coarser ones leave a list to walk in every bucket the
  // search reaches, finer ones leave the search reaching over more buckets to cover the same
  // distance. They are never finer than the vertex grid, which is as fine as the mesh itself gets
  const Real extent = (max_x - min_x) * (max_y - min_y);
  _boundary_cell = std::max(_grid_cell, std::sqrt(extent / _boundary_segments.size()));

  for (const auto segment : index_range(_boundary_segments))
  {
    const auto & [start, end] = _boundary_segments[segment];

    // Walking the segment in steps of half a bucket puts consecutive steps in the same bucket or in
    // neighboring ones, so every bucket the segment passes through neighbors one it is recorded in,
    // which is the reach the search below adds to the distance it has covered
    const auto steps =
        std::max(std::size_t(1),
                 static_cast<std::size_t>(std::ceil(2.0 * (end - start).norm() / _boundary_cell)));
    for (const auto step : make_range(steps + 1))
    {
      auto & bucket = _boundary_segment_grid[gridKey(start + (Real(step) / steps) * (end - start),
                                                     _boundary_cell)];
      // The steps of a straight segment reach a bucket in one run, so the last entry is the only
      // one that can already be this segment
      if (bucket.empty() || bucket.back() != segment)
        bucket.push_back(segment);
    }
  }
}

std::pair<Point, Point>
XYFrontalDelaunayGenerator::localFrame(const Point & point) const
{
  if (_cross_field)
    return _cross_field->crossFrame(point);

  mooseAssert(_boundary_cell > 0.0,
              "The grid over the boundary segments is built before any frame is taken from them.");

  const auto [center_i, center_j] = gridKey(point, _boundary_cell);

  // The nearest segment is the one of the lowest index at the smallest distance, whichever buckets
  // it is found in, so that the frame follows from the geometry and not from the order of the
  // search. The distances are compared squared, which orders them the same way
  Real nearest = std::numeric_limits<Real>::max();
  std::size_t nearest_segment = std::numeric_limits<std::size_t>::max();
  const auto search = [&](const long i, const long j)
  {
    const auto bucket = _boundary_segment_grid.find({i, j});
    if (bucket == _boundary_segment_grid.end())
      return;

    for (const auto segment : bucket->second)
    {
      const auto & [start, end] = _boundary_segments[segment];
      const Real distance = geom_utils::pointSegmentDistanceSq(point, start, end);
      if (distance < nearest || (distance == nearest && segment < nearest_segment))
      {
        nearest = distance;
        nearest_segment = segment;
      }
    }
  };

  // The buckets are searched a ring at a time, until the nearest segment found is closer than the
  // rings still to come can reach. A segment that has not been searched yet lies in a bucket more
  // than one ring out, and so no nearer than the ring before the one just searched
  for (long span = 0;; ++span)
  {
    if (span == 0)
      search(center_i, center_j);
    else
    {
      for (long i = center_i - span; i <= center_i + span; ++i)
      {
        search(i, center_j - span);
        search(i, center_j + span);
      }
      for (long j = center_j - span + 1; j <= center_j + span - 1; ++j)
      {
        search(center_i - span, j);
        search(center_i + span, j);
      }
    }

    const Real covered = std::max(0L, span - 1) * _boundary_cell;
    if (nearest < covered * covered)
      break;
  }

  const auto & [start, end] = _boundary_segments[nearest_segment];
  const Point tangent = (end - start).unit();

  return {tangent, Point(-tangent(1), tangent(0), 0.0)};
}

Real
XYFrontalDelaunayGenerator::metricDistance(const Point & first,
                                           const Point & second,
                                           const std::pair<Point, Point> & frame) const
{
  const Point offset = first - second;
  if (_metric == "L2")
    return offset.norm();

  return std::max(std::abs(offset * frame.first), std::abs(offset * frame.second));
}

bool
XYFrontalDelaunayGenerator::insideDomain(const Point & point) const
{
  if (!_outer_outline->contains(point))
    return false;

  for (const auto & outline : _hole_outlines)
    if (outline->contains(point))
      return false;

  return true;
}

void
XYFrontalDelaunayGenerator::addToGrid(const std::size_t vertex, const Point & point)
{
  _vertex_grid[gridKey(point, _grid_cell)].push_back(vertex);
}

bool
XYFrontalDelaunayGenerator::hasVertexWithin(const IncrementalDelaunay & delaunay,
                                            const Point & point,
                                            const Real distance,
                                            const std::pair<Point, Point> & frame) const
{
  // A LINF ball of the given radius reaches sqrt(2) times as far as an L2 one of the same radius,
  // so that is how far the buckets have to be searched for the vertices the metric then judges
  const Real reach = (_metric == "L2") ? distance : distance * std::sqrt(2.0);
  const long span = static_cast<long>(std::ceil(reach / _grid_cell));
  const auto [center_i, center_j] = gridKey(point, _grid_cell);

  // The buckets are sized on the smallest triangle the advance is asked for, so where the target is
  // coarser the reach spans many of them and all but a few are empty. Walking each row of the
  // search from the first bucket at or past its start leaves the empty ones unvisited, rather than
  // looked up one by one
  for (long i = center_i - span; i <= center_i + span; ++i)
    for (auto bucket = _vertex_grid.lower_bound({i, center_j - span});
         bucket != _vertex_grid.end() && bucket->first.first == i &&
         bucket->first.second <= center_j + span;
         ++bucket)
      for (const auto vertex : bucket->second)
        if (metricDistance(point, frontalToPoint(delaunay.point(vertex)), frame) < distance)
          return true;

  return false;
}

bool
XYFrontalDelaunayGenerator::placePoint(const IncrementalDelaunay & delaunay,
                                       const FrontEdge & edge,
                                       Point & point) const
{
  const Point start = frontalToPoint(delaunay.point(edge.start));
  const Point end = frontalToPoint(delaunay.point(edge.end));
  const Point midpoint = 0.5 * (start + end);

  const Real size = targetSize(targetArea(midpoint));

  // The triangle the front advances into is on the left of the edge, so the left normal points to
  // the side the new point goes on
  const Point along = end - start;
  const Real length = along.norm();
  const Point normal(-along(1) / length, along(0) / length, 0.0);

  // The frame only enters the LINF metric, so an L2 advance never asks for the cross field
  const std::pair<Point, Point> world_frame(Point(1.0, 0.0, 0.0), Point(0.0, 1.0, 0.0));
  const std::pair<Point, Point> frame = (_metric == "L2") ? world_frame : localFrame(midpoint);

  if (_metric == "L2")
  {
    // The apex at the target distance from both ends of the edge, or of the right isosceles
    // triangle on the edge once that edge is longer than sqrt(2) times the target size
    const Real half_length = 0.5 * length;
    point = midpoint + std::sqrt(std::max(size * size - half_length * half_length,
                                          half_length * half_length)) *
                           normal;
  }
  else
    point = frontalLinfCorner(start, end, normal, size, frame);

  if (!insideDomain(point))
    return false;

  // A point too close to a vertex that is already there would make a sliver; the front edge is
  // instead left to connect to that vertex, which the triangulation has already done
  return !hasVertexWithin(delaunay, point, _rejection_factor * size, frame);
}

std::vector<XYFrontalDelaunayGenerator::FrontEdge>
XYFrontalDelaunayGenerator::collectFront(
    const IncrementalDelaunay & delaunay,
    const std::vector<IncrementalDelaunay::Triangle> & triangles,
    const std::vector<bool> & inside) const
{
  // The circumradius answers for both the size and the shape of a triangle: it grows with the
  // triangle and it runs away as the triangle flattens
  std::vector<Real> excess(triangles.size(), 0.0);
  for (const auto t : index_range(triangles))
  {
    if (!inside[t])
      continue;

    const Point first = frontalToPoint(delaunay.point(triangles[t].vertices[0]));
    const Point second = frontalToPoint(delaunay.point(triangles[t].vertices[1]));
    const Point third = frontalToPoint(delaunay.point(triangles[t].vertices[2]));
    const Point centroid = (first + second + third) / 3.0;

    excess[t] = frontalCircumradius(first, second, third) /
                targetCircumradius(targetSize(targetArea(centroid)));
  }

  std::vector<FrontEdge> front;
  for (const auto t : index_range(triangles))
  {
    if (!inside[t] || excess[t] <= _size_tolerance)
      continue;

    const auto & triangle = triangles[t];
    for (const auto i : make_range(n_frontal_tri_sides))
    {
      // An edge between two triangles that both miss the target is behind the front, not on it
      const auto neighbor = triangle.neighbors[i];
      if (neighbor != IncrementalDelaunay::invalid_index && inside[neighbor] &&
          excess[neighbor] > _size_tolerance)
        continue;

      front.push_back({triangle.vertices[(i + 1) % n_frontal_tri_sides],
                       triangle.vertices[(i + 2) % n_frontal_tri_sides],
                       excess[t]});
    }
  }

  // Ties are broken by the vertices of the edge so that the same points are placed, in the same
  // order, from one run to the next
  std::sort(front.begin(),
            front.end(),
            [](const FrontEdge & a, const FrontEdge & b)
            {
              if (a.excess > b.excess)
                return true;
              if (b.excess > a.excess)
                return false;

              return IncrementalDelaunay::makeSegment(a.start, a.end) <
                     IncrementalDelaunay::makeSegment(b.start, b.end);
            });

  return front;
}

void
XYFrontalDelaunayGenerator::recordSplitBoundaryIds(const IncrementalDelaunay & delaunay,
                                                   const std::size_t vertex)
{
  // The vertex the insertion placed is the newest one, so it is the larger id of every segment it
  // is an end of, and the two halves are the only constrained segments it is an end of at all
  std::vector<std::size_t> ends;
  for (const auto & [first, second] : delaunay.constrainedSegments())
    if (second == vertex)
      ends.push_back(first);

  mooseAssert(ends.size() == 2,
              "A split replaces the segment the new vertex landed on by the two halves that vertex "
              "divides it into");

  const auto split = IncrementalDelaunay::makeSegment(ends.front(), ends.back());
  const auto recorded = _segment_boundary_ids.find(split);
  // A constrained segment carrying no boundary id belongs to no input boundary, which
  // buildTriangleMesh() reports; leaving the halves without one as well keeps that report
  if (recorded == _segment_boundary_ids.end())
    return;

  const boundary_id_type bcid = recorded->second;
  _segment_boundary_ids.erase(recorded);
  _segment_boundary_ids[IncrementalDelaunay::makeSegment(ends.front(), vertex)] = bcid;
  _segment_boundary_ids[IncrementalDelaunay::makeSegment(ends.back(), vertex)] = bcid;
}

void
XYFrontalDelaunayGenerator::advanceFront(IncrementalDelaunay & delaunay)
{
  // The front is walked over and over rather than rebuilt after every point, which is what keeps
  // the cost of the advance down. A pass that places nothing has nothing left to place: every
  // point is at least the rejection distance from every other, so only finitely many fit
  bool placed_any = true;
  while (placed_any)
  {
    placed_any = false;

    const auto triangles = delaunay.getTriangles();
    // The outer boundary was seeded first and counter-clockwise, so the domain is on the left of
    // the segment from vertex 0 to vertex 1
    const auto inside = frontalInsideTriangles(delaunay, triangles, 0, 1);

    for (const auto & edge : collectFront(delaunay, triangles, inside))
    {
      Point point;
      if (!placePoint(delaunay, edge, point))
        continue;

      const auto vertices_before = delaunay.numPoints();
      const auto segments_before = delaunay.constrainedSegments().size();
      const auto vertex = delaunay.insertPoint({point(0), point(1)});
      if (delaunay.numPoints() == vertices_before)
        continue;

      // A point that lands exactly on a constrained segment splits it, which leaves the boundary id
      // of that segment recorded against a segment the triangulation no longer has
      if (delaunay.constrainedSegments().size() != segments_before)
        recordSplitBoundaryIds(delaunay, vertex);

      addToGrid(vertex, point);
      placed_any = true;
    }
  }
}

std::unique_ptr<MeshBase>
XYFrontalDelaunayGenerator::buildTriangleMesh(const IncrementalDelaunay & delaunay)
{
  const auto triangles = delaunay.getTriangles();
  // The outer boundary was seeded first and counter-clockwise, so the domain is on the left of the
  // segment from vertex 0 to vertex 1
  const auto inside = frontalInsideTriangles(delaunay, triangles, 0, 1);

  auto mesh = buildReplicatedMesh(2);

  // Adding the nodes in vertex order rather than as the triangles reach them keeps the node
  // numbering of the output tied to the order the points were placed in
  std::set<std::size_t> used_vertices;
  for (const auto t : index_range(triangles))
    if (inside[t])
      used_vertices.insert(triangles[t].vertices.begin(), triangles[t].vertices.end());

  std::map<std::size_t, Node *> nodes;
  for (const auto vertex : used_vertices)
    nodes[vertex] = mesh->add_point(frontalToPoint(delaunay.point(vertex)));

  auto & boundary_info = mesh->get_boundary_info();
  for (const auto t : index_range(triangles))
  {
    if (!inside[t])
      continue;

    const auto & triangle = triangles[t];
    Elem * const elem = mesh->add_elem(Elem::build(libMesh::ElemType::TRI3));
    for (const auto k : make_range(n_frontal_tri_sides))
      elem->set_node(k, libmesh_map_find(nodes, triangle.vertices[k]));
    elem->subdomain_id() = 0;

    for (const auto side : make_range(n_frontal_tri_sides))
    {
      // Side s of a triangle runs from vertex s to vertex s + 1, which is the edge the
      // triangulation holds opposite vertex s + 2
      const auto neighbor = triangle.neighbors[(side + 2) % n_frontal_tri_sides];
      if (neighbor != IncrementalDelaunay::invalid_index && inside[neighbor])
        continue;

      const auto first = triangle.vertices[side];
      const auto second = triangle.vertices[(side + 1) % n_frontal_tri_sides];
      const auto bcid = _segment_boundary_ids.find(IncrementalDelaunay::makeSegment(first, second));
      if (bcid == _segment_boundary_ids.end())
        mooseError("A side of the triangulation lies on the boundary of the domain without lying "
                   "on any of the input boundaries, which happens when a point of "
                   "'interior_points' falls on the outer boundary or on a hole boundary.");

      boundary_info.add_side(elem, side, bcid->second);
    }
  }

  mesh->prepare_for_use();

  return mesh;
}

std::unique_ptr<MeshBase>
XYFrontalDelaunayGenerator::generate()
{
  std::unique_ptr<MeshBase> boundary_mesh = std::move(_bdy_ptr);

  std::vector<std::unique_ptr<MeshBase>> hole_meshes(_hole_ptrs.size());
  for (const auto hole_i : index_range(_hole_ptrs))
    hole_meshes[hole_i] = std::move(*_hole_ptrs[hole_i]);

  // The advance places one point at a time against the whole triangulation, which no process holds
  // a part of
  if (!boundary_mesh->is_replicated())
    mooseError("XYFrontalDelaunayGenerator is not implemented for distributed meshes");

  // The outer boundary is only known once the mesh knows which of its sides face outward
  if (!boundary_mesh->is_prepared())
    boundary_mesh->prepare_for_use();

  for (const auto & elem : boundary_mesh->element_ptr_range())
    if (elem->default_order() != libMesh::FIRST)
      paramError("boundary",
                 "Element ",
                 elem->id(),
                 " is a ",
                 libMesh::Utility::enum_to_string(elem->type()),
                 " element. Only first order boundary elements are supported, because this mesh "
                 "generator produces TRI3 elements.");

  MeshTriangulationUtils::XYDelaunayOptions opts;
  fillDelaunayOptions(opts);

  _outer_outline = std::make_unique<libMesh::TriangulatorInterface::MeshedHole>(
      *boundary_mesh, MeshTriangulationUtils::outerBoundaryIds(*this, *boundary_mesh, opts));

  std::vector<bool> holes_with_midpoints(hole_meshes.size());
  _hole_outlines.reserve(hole_meshes.size());
  for (const auto hole_i : index_range(hole_meshes))
  {
    if (!hole_meshes[hole_i]->is_prepared())
      hole_meshes[hole_i]->prepare_for_use();

    _hole_outlines.push_back(
        std::make_unique<libMesh::TriangulatorInterface::MeshedHole>(*hole_meshes[hole_i]));
    holes_with_midpoints[hole_i] = _hole_outlines.back()->n_midpoints();

    if (holes_with_midpoints[hole_i] && hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
      paramError("stitch_holes",
                 "Cannot stitch a quadratic element hole to the first order triangles this mesh "
                 "generator produces. Please reduce the order of the hole inputs.");
  }

  _background_mesh = buildBackgroundMesh(*boundary_mesh, hole_meshes, opts);
  _background_mesh->prepare_for_use();
  _background_locator = _background_mesh->sub_point_locator();
  _background_locator->enable_out_of_mesh_mode();

  Real background_area = 0.0;
  for (const auto & elem : _background_mesh->element_ptr_range())
    background_area += elem->volume();
  _background_mean_area = background_area / _background_mesh->n_elem();

  if (!_desired_area_func.empty())
    _area_function = std::make_unique<libMesh::ParsedFunction<Real>>(_desired_area_func);

  // The buckets of the rejection rule are sized on the smallest triangle the advance is asked for,
  // so that a search of the buckets around a point covers the distance the rule needs
  _grid_cell = std::numeric_limits<Real>::max();
  for (const auto & elem : _background_mesh->element_ptr_range())
    _grid_cell = std::min(_grid_cell, targetSize(targetArea(elem->vertex_average())));

  if (_metric == "LINF" && _orientation == "CROSS_FIELD")
  {
    _cross_field = std::make_unique<CrossFieldSolver>(*_background_mesh,
                                                      boundaryTangentAngles(*_background_mesh));
    _cross_field->solve();
  }

  // The outer boundary is walked counter-clockwise so that the domain is on the left of its first
  // segment, which is where the triangles in the domain are then found from
  std::vector<Point> outer_loop;
  for (const auto i : make_range(_outer_outline->n_points()))
    outer_loop.push_back(_outer_outline->point(i));
  frontalCanonicalizeLoop(outer_loop);

  std::vector<Point> points;
  std::vector<IncrementalDelaunay::Segment> segments;
  appendLoop(outer_loop, _refine_bdy, _add_nodes_per_boundary_segment, 0, points, segments);

  for (const auto hole_i : index_range(_hole_outlines))
  {
    std::vector<Point> hole_loop;
    for (const auto i : make_range(_hole_outlines[hole_i]->n_points()))
      hole_loop.push_back(_hole_outlines[hole_i]->point(i));
    frontalCanonicalizeLoop(hole_loop);

    const bool refine = (hole_i >= _refine_holes.size() || _refine_holes[hole_i]);
    appendLoop(hole_loop, refine, 0, static_cast<boundary_id_type>(hole_i + 1), points, segments);
  }

  // The frame of the nearest boundary segment is only asked for once every segment has been seeded,
  // and only when no cross field answers for it
  if (_metric == "LINF" && !_cross_field)
    buildBoundarySegmentGrid();

  // A point on a boundary would split the segment it lies on, which would leave the output mesh
  // with a side that belongs to no input boundary
  for (const auto & interior_point : _interior_points)
    if (insideDomain(interior_point))
      points.push_back(interior_point);

  std::vector<IncrementalDelaunay::Point2D> plane_points;
  plane_points.reserve(points.size());
  for (const auto & point : points)
    plane_points.push_back({point(0), point(1)});

  IncrementalDelaunay delaunay;
  delaunay.initialize(plane_points, segments);

  for (const auto vertex : make_range(delaunay.numPoints()))
    addToGrid(vertex, frontalToPoint(delaunay.point(vertex)));

  advanceFront(delaunay);

  auto mesh = buildTriangleMesh(delaunay);

  MeshTriangulationUtils::finalizeTriangulation(
      *this, dynamic_cast<UnstructuredMesh &>(*mesh), hole_meshes, holes_with_midpoints, opts);

  return mesh;
}

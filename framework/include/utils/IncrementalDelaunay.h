//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

/**
 * Constrained Delaunay triangulation of a set of points in the plane, built one point at a time.
 *
 * Points are inserted with the Bowyer-Watson construction: the triangle containing the new point is
 * found by walking the triangulation, the cavity of triangles whose circumcircle contains the point
 * is deleted, and the point is connected to the boundary of that cavity. Constrained segments are
 * edges that the triangulation is required to contain; the cavity never grows across one, and a
 * segment that is not an edge once the points are in is recovered by retriangulating the strip of
 * triangles it crosses.
 *
 * After every insertion no vertex lies strictly inside the circumcircle of any triangle, except
 * where a constrained segment separates the two. Cocircular and collinear configurations are
 * resolved consistently rather than arbitrarily, so the same input always gives the same
 * triangulation. Every geometric decision is made with the exact predicates in
 * framework/contrib/predicates, so no tolerance enters the algorithm.
 *
 * The triangulation carries no mesh types: points are plain coordinate pairs and triangles are
 * plain index triples, so it can be exercised on its own.
 *
 * Internally the point set is padded with the three vertices of a bounding triangle that encloses
 * everything, which is what lets the cavity of every insertion be a closed polygon. Those three
 * vertices are not part of the caller's point set and never appear in the public interface: the
 * vertex ids used here run from 0 to numPoints() - 1 in the order the points were added, and
 * getTriangles() drops the triangles that touch the bounding triangle.
 */
class IncrementalDelaunay
{
public:
  /// Sentinel for a vertex, triangle or neighbor that does not exist
  static constexpr std::size_t invalid_index = std::numeric_limits<std::size_t>::max();

  /// A point of the triangulation, held as plain coordinates
  struct Point2D
  {
    double x;
    double y;
  };

  /**
   * A triangle of the triangulation.
   *
   * The three vertices are ordered counter-clockwise. Entry i of neighbors is the triangle across
   * the edge opposite vertices[i], that is the edge from vertices[(i + 1) % 3] to
   * vertices[(i + 2) % 3], or invalid_index when no triangle lies across that edge.
   */
  struct Triangle
  {
    std::array<std::size_t, 3> vertices;
    std::array<std::size_t, 3> neighbors;
  };

  /// A constrained segment, held as a vertex id pair with the smaller id first
  using Segment = std::pair<std::size_t, std::size_t>;

  /// @return A vertex id pair with the smaller id first, the form constrained segments are held in
  static Segment makeSegment(std::size_t v0, std::size_t v1);

  /**
   * Triangulates points and recovers every entry of segments as an edge of the result. Points are
   * inserted in the order given, so vertex id i refers to points[i]. Any later insertPoint() must
   * lie within the region this call encloses, which is the bounding box of points grown by a large
   * multiple of its own size; the interior points a frontal advance adds always do.
   * @param points The points to triangulate, which must not contain two identical points
   * @param segments The segments the triangulation is required to contain, as vertex id pairs in
   * either order
   */
  void initialize(const std::vector<Point2D> & points, const std::vector<Segment> & segments);

  /**
   * Inserts a point, restoring the constrained Delaunay property around it.
   * @param p The point to insert
   * @return The vertex id of p. A point identical to an existing vertex is not inserted a second
   * time; the id of that vertex is returned and the triangulation is left alone. A point that falls
   * exactly on a constrained segment splits it, replacing that segment by the two halves the point
   * divides it into.
   */
  std::size_t insertPoint(const Point2D & p);

  /**
   * Makes the segment between two vertices an edge of the triangulation and records it as
   * constrained, so that later insertions neither remove nor cross it. Doing this to a segment that
   * is already an edge only records the constraint.
   *
   * A segment that runs exactly through a third vertex cannot be a single edge, and is recorded as
   * the pieces that vertex divides it into rather than as itself.
   * @param v0 The vertex id of one end of the segment
   * @param v1 The vertex id of the other end of the segment
   */
  void insertSegment(std::size_t v0, std::size_t v1);

  /// @return The number of points in the triangulation, whose vertex ids run from 0 to this minus 1
  std::size_t numPoints() const;

  /// @return The coordinates of the vertex with the given id
  const Point2D & point(std::size_t id) const;

  /**
   * @return The triangles covering the convex hull of the points, counter-clockwise, ordered so
   * that the same input always gives the same list. The neighbor entries index into this same list
   * and are invalid_index on the hull, where no triangle lies across the edge.
   */
  std::vector<Triangle> getTriangles() const;

  /// @return Whether the segment between two vertices is constrained, given its ends in either order
  bool isConstrainedSegment(std::size_t v0, std::size_t v1) const;

  /// @return The constrained segments, each with the smaller vertex id first
  const std::set<Segment> & constrainedSegments() const { return _constraints; }

  /**
   * Checks everything this class promises: that every triangle is counter-clockwise, that the
   * neighbor entries agree with each other, that every constrained segment is an edge, and that
   * every edge that is not constrained is locally Delaunay. A triangulation whose unconstrained
   * edges are all locally Delaunay is a constrained Delaunay triangulation, so an empty result is
   * the whole contract and not a sample of it.
   * @return One entry describing each violation found, empty if there are none
   */
  std::vector<std::string> checkInvariants() const;

  /**
   * Tests every vertex against the circumcircle of every triangle. This ignores constrained
   * segments, which a vertex is allowed to sit behind, so it is the right criterion only for a
   * triangulation with no constrained segments; use checkInvariants() otherwise.
   * @return One entry describing each vertex that lies strictly inside a circumcircle, empty if
   * there are none
   */
  std::vector<std::string> checkEmptyCircumcircle() const;

private:
  /// The number of bounding triangle vertices padding the front of the vertex list
  static constexpr std::size_t _num_bounding = 3;

  /// How far the bounding triangle reaches beyond the points, as a multiple of their extent
  static constexpr double _bounding_reach = 1000.0;

  /// @return The two coordinates of an internal vertex, in the layout the exact predicates take
  const double * xy(std::size_t v) const { return &_vertices[v].x; }

  /// @return Whether an internal vertex belongs to the bounding triangle rather than to the caller
  static bool isBounding(std::size_t v) { return v < _num_bounding; }

  /// @return The internal vertex id of a caller vertex id, which must be in range
  std::size_t toInternal(std::size_t id) const;

  /// @return The caller vertex id of an internal vertex id, which must not be a bounding vertex
  std::size_t toCaller(std::size_t v) const;

  /// @return Whether the edge between two internal vertices is a constrained segment
  bool isConstrainedEdge(std::size_t v0, std::size_t v1) const;

  /// @return The position of an internal vertex within a triangle
  unsigned int localVertexIndex(std::size_t t, std::size_t v) const;

  /// @return The position within a triangle of the edge opposite the given edge's vertex
  unsigned int localEdgeIndex(std::size_t t, const Segment & edge) const;

  /// @return Whether a point lies inside or on the boundary of a triangle
  bool containsPoint(std::size_t t, const Point2D & p) const;

  /**
   * Finds the triangle a point falls in by walking from the triangle the last insertion produced,
   * which keeps the search short when successive points are close together. Falls back to a scan of
   * every triangle if the walk leaves the triangulation or fails to settle.
   * @return The triangle containing p, or invalid_index if p is outside the triangulation
   */
  std::size_t locate(const Point2D & p) const;

  /**
   * Collects the triangles that have to make way for a new vertex, starting from the triangle it
   * falls in and spreading to every neighbor whose circumcircle contains it. Constrained segments
   * stop the spread. A neighbor is also taken in when the shared edge would otherwise produce a
   * triangle of zero or negative area, which is what keeps the cavity star shaped about the vertex.
   * @param seed The triangle the new vertex falls in
   * @param v_new The internal vertex id of the new vertex
   * @param cavity Filled with the triangles to remove
   */
  void growCavity(std::size_t seed, std::size_t v_new, std::set<std::size_t> & cavity) const;

  /**
   * Swaps one triangulation of a region for another, reusing the slots of the triangles it removes
   * and rebuilding the neighbor entries both inside the region and along its boundary.
   * @param removed The triangles covering the region, in any order
   * @param added The vertices of the triangles to cover it with, counter-clockwise
   * @return The slots the added triangles were placed in
   */
  std::vector<std::size_t> retriangulate(const std::vector<std::size_t> & removed,
                                         const std::vector<std::array<std::size_t, 3>> & added);

  /**
   * Triangulates a polygon whose vertices are all visible from one of its edges, which is the shape
   * a recovered segment leaves on either side of itself. The polygon runs counter-clockwise from
   * v_start to v_end and back along chain[first] to chain[last - 1]. Splitting it at the vertex
   * whose circumcircle holds no other and recursing on the two halves makes it Delaunay.
   * @param v_start The internal vertex at the start of the edge every vertex sees
   * @param v_end The internal vertex at the end of that edge
   * @param chain The internal vertices of the rest of the polygon, in counter-clockwise order
   * @param first The position in chain where this polygon begins
   * @param last One past the position in chain where this polygon ends
   * @param triangles Appended with the vertices of the triangles covering the polygon
   */
  void triangulatePseudopolygon(std::size_t v_start,
                                std::size_t v_end,
                                const std::vector<std::size_t> & chain,
                                std::size_t first,
                                std::size_t last,
                                std::vector<std::array<std::size_t, 3>> & triangles) const;

  /// @return Whether v_mid lies strictly between v_first and v_last, which must all be collinear
  bool isStrictlyBetween(std::size_t v_first, std::size_t v_mid, std::size_t v_last) const;

  /// @return A name for an internal vertex, for the messages the check methods return
  std::string vertexName(std::size_t v) const;

  /// The bounding triangle vertices followed by the caller's points
  std::vector<Point2D> _vertices;

  /// The triangles, every one of them live and counter-clockwise
  std::vector<Triangle> _triangles;

  /// One triangle touching each vertex of _vertices, which is where a walk around it starts
  std::vector<std::size_t> _vertex_triangle;

  /// The constrained segments, in caller vertex ids with the smaller id first
  std::set<Segment> _constraints;

  /// A triangle the last insertion produced, which is where the next point walk starts
  std::size_t _last_triangle = invalid_index;
};

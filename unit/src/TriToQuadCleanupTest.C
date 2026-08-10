//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"
#include "TriToQuadGenerator.h"

#include "libmesh/int_range.h"
#include "libmesh/point.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

using QuadCorners = TriToQuadGenerator::QuadCorners;

namespace
{

/// Tolerance for the areas and the distances, which are built from divisions
constexpr Real tol = 1e-12;

/// A side of a filling, as the indices of its two ends in the order a quadrilateral runs along it
using DirectedSide = std::pair<unsigned int, unsigned int>;

/// A side of a patch, as the indices of its two ends in increasing order
using SideKey = std::pair<unsigned int, unsigned int>;

/**
 * Twice the signed area of a polygon in the xy plane, which is positive when its corners are
 * ordered counter-clockwise. Computed here rather than taken from the generator so that the tests
 * check their own input against the ordering the generator requires.
 * @param polygon_points the corners of the polygon, in order around it
 * @return twice the signed area
 */
template <typename T>
Real
twiceSignedArea(const T & polygon_points)
{
  Real twice_area = 0.0;
  for (const auto k : index_range(polygon_points))
  {
    const Point & current = polygon_points[k];
    const Point & next = polygon_points[(k + 1) % polygon_points.size()];
    twice_area += current(0) * next(1) - next(0) * current(1);
  }

  return twice_area;
}

/**
 * The corners of one quadrilateral of a filling, taken out of the point list its indices run into.
 * @param points the point list the filling indexes into
 * @param quad the four indices of the quadrilateral
 * @return the four corners
 */
std::array<Point, 4>
quadPoints(const std::vector<Point> & points, const QuadCorners & quad)
{
  return {points[quad[0]], points[quad[1]], points[quad[2]], points[quad[3]]};
}

/**
 * The number of quadrilaterals of a filling that reach one point of its point list.
 * @param quads the quadrilaterals of the filling, as indices into a point list
 * @param index the index of the point
 * @return how many quadrilaterals hold that index
 */
std::size_t
nQuadsUsing(const std::vector<QuadCorners> & quads, const unsigned int index)
{
  std::size_t n_quads = 0;
  for (const auto & quad : quads)
    if (std::count(quad.begin(), quad.end(), index))
      ++n_quads;

  return n_quads;
}

/**
 * The outer boundary of a filling, which is the sides that only one of its quadrilaterals uses. A
 * conformal filling of counter-clockwise quadrilaterals runs along every other side exactly twice,
 * once from each end, and never runs along a side twice the same way round, which is checked here
 * because a filling that did would be folded over rather than conformal.
 * @param quads the quadrilaterals of the filling, as indices into a point list
 * @param boundary filled with the directed sides that only one quadrilateral uses
 * @return whether no two quadrilaterals run along a side the same way round
 */
bool
boundarySides(const std::vector<QuadCorners> & quads, std::set<DirectedSide> & boundary)
{
  std::set<DirectedSide> directed_sides;
  for (const auto & quad : quads)
    for (const auto k : index_range(quad))
      if (!directed_sides.insert({quad[k], quad[(k + 1) % quad.size()]}).second)
        return false;

  boundary.clear();
  for (const auto & side : directed_sides)
    if (!directed_sides.count({side.second, side.first}))
      boundary.insert(side);

  return true;
}

/**
 * The point list a subdivision template indexes into, built for one element on its own.
 * @param corner_points the corners of the element, in counter-clockwise order
 * @return the corners, then the midpoint of the side running from each corner to the next, then
 * the centroid
 */
std::vector<Point>
templatePoints(const std::vector<Point> & corner_points)
{
  const std::size_t n_corners = corner_points.size();

  std::vector<Point> points = corner_points;
  for (const auto s : index_range(corner_points))
    points.push_back((corner_points[s] + corner_points[(s + 1) % n_corners]) / 2.0);

  Point centroid;
  for (const auto & corner : corner_points)
    centroid += corner;
  points.push_back(centroid / Real(n_corners));

  return points;
}

/**
 * Check the shape of a subdivision template: one quadrilateral per corner of the element, each of
 * them reaching the centroid once, and the midpoint of each side reached by the two quadrilaterals
 * on the corners of that side and by no others, which is what makes the filling conformal inside
 * the element.
 * @param quads the quadrilaterals of the template
 * @param n_corners the number of corners of the element the template is for
 */
void
checkSubdivisionTemplate(const std::vector<QuadCorners> & quads, const unsigned int n_corners)
{
  ASSERT_EQ(std::size_t(n_corners), quads.size());

  const unsigned int centroid = 2 * n_corners;
  for (const auto k : make_range(n_corners))
  {
    EXPECT_EQ(1, std::count(quads[k].begin(), quads[k].end(), centroid)) << "quadrilateral " << k;

    // One quadrilateral per corner, so no two of them meet on a corner of the element
    EXPECT_EQ(std::size_t(1), nQuadsUsing(quads, k)) << "corner " << k;

    // Side k runs from corner k to corner k + 1, so its midpoint belongs to those two corners'
    // quadrilaterals and to no other
    const unsigned int midpoint = n_corners + k;
    const unsigned int next_corner = (k + 1) % n_corners;
    EXPECT_EQ(std::size_t(2), nQuadsUsing(quads, midpoint)) << "side " << k;
    EXPECT_EQ(1, std::count(quads[k].begin(), quads[k].end(), midpoint)) << "side " << k;
    EXPECT_EQ(1, std::count(quads[next_corner].begin(), quads[next_corner].end(), midpoint))
        << "side " << k;
  }
}

/**
 * Check that a template covers the element it is for exactly once over, with every quadrilateral
 * of it wound the same way round as the element.
 * @param corner_points the corners of the element, in counter-clockwise order
 * @param quads the quadrilaterals of the template
 */
void
checkTiling(const std::vector<Point> & corner_points, const std::vector<QuadCorners> & quads)
{
  const auto points = templatePoints(corner_points);
  ASSERT_EQ(2 * corner_points.size() + 1, points.size());

  Real twice_filled_area = 0.0;
  for (const auto k : index_range(quads))
  {
    // A quadrilateral wound the other way round, which is an inverted element, would come out
    // negative here
    const Real twice_quad_area = twiceSignedArea(quadPoints(points, quads[k]));
    EXPECT_GT(twice_quad_area, 0.0) << "quadrilateral " << k;
    twice_filled_area += twice_quad_area;
  }

  EXPECT_NEAR(twiceSignedArea(corner_points), twice_filled_area, tol);
}

/**
 * The key that identifies a side of a patch, which is the indices of its two ends in increasing
 * order, so that the two elements on either side of it key that side the same way.
 * @param end_1 the index of the point at one end of the side
 * @param end_2 the index of the point at the other end of the side
 * @return the side key
 */
SideKey
sideKey(const unsigned int end_1, const unsigned int end_2)
{
  return std::make_pair(std::min(end_1, end_2), std::max(end_1, end_2));
}

/**
 * Split one element of a patch with the template for its number of corners, turning the indices of
 * that template into indices into the point list of the whole patch. Midpoints are keyed on the
 * side they split, the way the generator keys them, so the two elements on either side of a shared
 * side reach the same midpoint rather than one each.
 * @param corner_ids the corners of the element, as indices into the point list of the patch, in
 * counter-clockwise order
 * @param points the point list of the patch, which the midpoints and the centroid are added to
 * @param midpoints the midpoints made so far, keyed on the side that they split
 * @return the quadrilaterals the element is split into, as indices into the point list of the patch
 */
std::vector<QuadCorners>
subdivideElement(const std::vector<unsigned int> & corner_ids,
                 std::vector<Point> & points,
                 std::map<SideKey, unsigned int> & midpoints)
{
  const std::size_t n_corners = corner_ids.size();

  std::vector<unsigned int> patch_indices = corner_ids;
  for (const auto s : index_range(corner_ids))
  {
    const unsigned int first = corner_ids[s];
    const unsigned int second = corner_ids[(s + 1) % n_corners];

    const auto it = midpoints.find(sideKey(first, second));
    if (it != midpoints.end())
    {
      patch_indices.push_back(it->second);
      continue;
    }

    const Point midpoint_position = (points[first] + points[second]) / 2.0;
    points.push_back(midpoint_position);

    const auto midpoint = cast_int<unsigned int>(points.size() - 1);
    midpoints.emplace(sideKey(first, second), midpoint);
    patch_indices.push_back(midpoint);
  }

  Point centroid;
  for (const auto corner : corner_ids)
    centroid += points[corner];
  points.push_back(centroid / Real(n_corners));
  patch_indices.push_back(cast_int<unsigned int>(points.size() - 1));

  const auto quad_template = n_corners == 3 ? TriToQuadGenerator::triSubdivisionTemplate()
                                            : TriToQuadGenerator::quadSubdivisionTemplate();

  std::vector<QuadCorners> quads;
  for (const auto & quad : quad_template)
    quads.push_back(QuadCorners{patch_indices[quad[0]],
                                patch_indices[quad[1]],
                                patch_indices[quad[2]],
                                patch_indices[quad[3]]});

  return quads;
}

} // namespace

/**
 * A triangle is split into one quadrilateral per corner, each of them running from that corner
 * along one side to its midpoint, in to the centroid and back out to the midpoint of the other
 * side the corner is an end of.
 */
TEST(TriToQuadCleanupTest, triTemplateIsOneQuadPerCorner)
{
  const auto quads = TriToQuadGenerator::triSubdivisionTemplate();

  // The point list holds the three corners at 0 to 2, the midpoint of the side running from corner
  // s to corner s + 1 at 3 + s, and the centroid at 6
  const std::vector<QuadCorners> expected_quads = {
      QuadCorners{0, 3, 6, 5}, QuadCorners{1, 4, 6, 3}, QuadCorners{2, 5, 6, 4}};
  EXPECT_EQ(expected_quads, quads);

  checkSubdivisionTemplate(quads, 3u);
}

/**
 * A quadrilateral is split the same way, one quadrilateral per corner, which makes four of them.
 */
TEST(TriToQuadCleanupTest, quadTemplateIsOneQuadPerCorner)
{
  const auto quads = TriToQuadGenerator::quadSubdivisionTemplate();

  // The point list holds the four corners at 0 to 3, the midpoint of the side running from corner
  // s to corner s + 1 at 4 + s, and the centroid at 8
  const std::vector<QuadCorners> expected_quads = {QuadCorners{0, 4, 8, 7},
                                                   QuadCorners{1, 5, 8, 4},
                                                   QuadCorners{2, 6, 8, 5},
                                                   QuadCorners{3, 7, 8, 6}};
  EXPECT_EQ(expected_quads, quads);

  checkSubdivisionTemplate(quads, 4u);
}

/**
 * Both templates cover the element they are for exactly once over, and every quadrilateral they
 * make is wound counter-clockwise like the element, so none of them comes out inverted. The two
 * elements below are deliberately lopsided, which makes the sub quadrilaterals come out at
 * different sizes and so leaves nothing for a mixed up index to hide behind.
 */
TEST(TriToQuadCleanupTest, templatesTileTheirElement)
{
  const std::vector<Point> triangle = {Point(0.0, 0.0), Point(2.0, 0.0), Point(0.5, 1.5)};
  ASSERT_GT(twiceSignedArea(triangle), 0.0);
  checkTiling(triangle, TriToQuadGenerator::triSubdivisionTemplate());

  const std::vector<Point> quadrilateral = {
      Point(0.0, 0.0), Point(3.0, 0.0), Point(2.0, 2.0), Point(0.0, 1.0)};
  ASSERT_GT(twiceSignedArea(quadrilateral), 0.0);
  checkTiling(quadrilateral, TriToQuadGenerator::quadSubdivisionTemplate());
}

/**
 * Neither template takes an argument, so the same call has to give the same quadrilaterals in the
 * same order every time. The mesh the generator writes is only reproducible from one run to the
 * next if it does.
 */
TEST(TriToQuadCleanupTest, templatesAreDeterministic)
{
  const auto tri_quads = TriToQuadGenerator::triSubdivisionTemplate();
  const auto quad_quads = TriToQuadGenerator::quadSubdivisionTemplate();

  for (const auto call : make_range(4u))
  {
    EXPECT_EQ(tri_quads, TriToQuadGenerator::triSubdivisionTemplate()) << "call " << call;
    EXPECT_EQ(quad_quads, TriToQuadGenerator::quadSubdivisionTemplate()) << "call " << call;
  }
}

/**
 * Subdividing every element of a mixed patch leaves it conformal across the elements and not only
 * inside each one. The patch is two quadrilaterals side by side with a triangle on the end of the
 * strip: A and B share the side from node 1 to node 4, B and C the side from node 2 to node 5.
 *
 * The midpoint of a side is keyed on that side here, the way the generator keys it, so a shared
 * side has one midpoint that both of its elements are handed. What the templates have to get right
 * is the rest: the walk around the outside of a subdivided element has to step through the midpoint
 * of each of its sides and through nothing else, or the two elements on a shared side would not
 * meet along it. A check made one element at a time would say nothing about that.
 */
TEST(TriToQuadCleanupTest, mixedPatchStaysConformal)
{
  std::vector<Point> points = {Point(0.0, 0.0),
                               Point(1.0, 0.0),
                               Point(2.0, 0.0),
                               Point(0.0, 1.0),
                               Point(1.2, 1.1),
                               Point(2.0, 1.0),
                               Point(3.0, 0.5)};
  const std::size_t n_nodes = points.size();

  const std::vector<std::vector<unsigned int>> parents = {{0, 1, 4, 3}, {1, 2, 5, 4}, {2, 6, 5}};

  std::map<SideKey, unsigned int> midpoints;
  std::vector<std::vector<QuadCorners>> parent_quads;
  Real twice_patch_area = 0.0;
  for (const auto & corner_ids : parents)
  {
    std::vector<Point> corner_points;
    for (const auto corner : corner_ids)
      corner_points.push_back(points[corner]);

    ASSERT_GT(twiceSignedArea(corner_points), 0.0);
    twice_patch_area += twiceSignedArea(corner_points);

    parent_quads.push_back(subdivideElement(corner_ids, points, midpoints));
  }

  std::vector<QuadCorners> patch_quads;
  for (const auto & quads : parent_quads)
    patch_quads.insert(patch_quads.end(), quads.begin(), quads.end());

  // Four quadrilaterals out of each of the two quadrilaterals and three out of the triangle
  EXPECT_EQ(std::size_t(11), patch_quads.size());

  // The patch has nine distinct sides, two of which are shared, so a midpoint made once per element
  // rather than once per side would leave eleven midpoints and two extra points
  EXPECT_EQ(std::size_t(9), midpoints.size());
  EXPECT_EQ(n_nodes + midpoints.size() + parents.size(), points.size());

  // No two points of the patch sit on top of each other, so no side carries two midpoints that only
  // happen to be in the same place
  for (const auto k : index_range(points))
    for (const auto l : make_range(k + 1, points.size()))
      EXPECT_GT((points[k] - points[l]).norm(), tol) << "points " << k << " and " << l;

  // The midpoint of a shared side is one point, and two quadrilaterals from the element on each
  // side of it reach that same point
  const unsigned int shared_by_a_and_b = libmesh_map_find(midpoints, sideKey(1, 4));
  EXPECT_EQ(std::size_t(2), nQuadsUsing(parent_quads[0], shared_by_a_and_b));
  EXPECT_EQ(std::size_t(2), nQuadsUsing(parent_quads[1], shared_by_a_and_b));

  const unsigned int shared_by_b_and_c = libmesh_map_find(midpoints, sideKey(2, 5));
  EXPECT_EQ(std::size_t(2), nQuadsUsing(parent_quads[1], shared_by_b_and_c));
  EXPECT_EQ(std::size_t(2), nQuadsUsing(parent_quads[2], shared_by_b_and_c));

  Real twice_filled_area = 0.0;
  for (const auto k : index_range(patch_quads))
  {
    const Real twice_quad_area = twiceSignedArea(quadPoints(points, patch_quads[k]));
    EXPECT_GT(twice_quad_area, 0.0) << "quadrilateral " << k;
    twice_filled_area += twice_quad_area;
  }

  EXPECT_NEAR(twice_patch_area, twice_filled_area, tol);

  // Nothing hangs anywhere in the patch: every side of the filling is either shared by two of the
  // eleven quadrilaterals, from opposite ends, or on the boundary of the patch, and the boundary is
  // every original side of the patch broken at its midpoint and nowhere else
  std::set<DirectedSide> boundary;
  ASSERT_TRUE(boundarySides(patch_quads, boundary));

  const std::vector<DirectedSide> patch_boundary = {
      {0, 1}, {1, 2}, {2, 6}, {6, 5}, {5, 4}, {4, 3}, {3, 0}};
  std::set<DirectedSide> expected_boundary;
  for (const auto & side : patch_boundary)
  {
    const unsigned int midpoint = libmesh_map_find(midpoints, sideKey(side.first, side.second));
    expected_boundary.insert({side.first, midpoint});
    expected_boundary.insert({midpoint, side.second});
  }

  EXPECT_EQ(expected_boundary, boundary);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"
#include "IncrementalDelaunay.h"
#include "predicates.h"

#include "libmesh/int_range.h"

#include <cstddef>
#include <set>
#include <string>
#include <vector>

// IncrementalDelaunay carries no libMesh types; this is only for make_range() and index_range(),
// which no other header of this file brings in.
using namespace libMesh;

using Point2D = IncrementalDelaunay::Point2D;
using Segment = IncrementalDelaunay::Segment;
using Triangle = IncrementalDelaunay::Triangle;

namespace
{

/**
 * Turns the list of violations one of the check methods returns into a gtest result, so that a
 * failure names what went wrong instead of only reporting that something did.
 * @param what the check that was run, and where in the test it was run
 * @param violations the descriptions the check returned, empty when it found nothing
 * @return success when there are no violations, otherwise a failure listing all of them
 */
::testing::AssertionResult
noViolations(const std::string & what, const std::vector<std::string> & violations)
{
  if (violations.empty())
    return ::testing::AssertionSuccess();

  auto failure = ::testing::AssertionFailure();
  failure << what << " reported " << violations.size() << " violation(s):";
  for (const auto & violation : violations)
    failure << "\n  " << violation;

  return failure;
}

/**
 * The sign of the exact orientation determinant of three points of the triangulation: positive when
 * they run counter-clockwise, negative when they run clockwise and zero when they are collinear.
 * The predicate takes each point as two adjacent doubles, so the coordinates are copied out rather
 * than the Point2D passed through, which leaves the test free of any assumption about its layout.
 * @param tri the triangulation the points belong to
 * @param v0 the first point
 * @param v1 the second point
 * @param v2 the third point
 * @return the sign of the determinant
 */
int
orientationSign(const IncrementalDelaunay & tri,
                const std::size_t v0,
                const std::size_t v1,
                const std::size_t v2)
{
  const double a[2] = {tri.point(v0).x, tri.point(v0).y};
  const double b[2] = {tri.point(v1).x, tri.point(v1).y};
  const double c[2] = {tri.point(v2).x, tri.point(v2).y};

  const auto determinant = moose_orient2d(a, b, c);
  if (determinant > 0.0)
    return 1;
  if (determinant < 0.0)
    return -1;

  return 0;
}

/**
 * Whether the segments between two pairs of points cross at a point interior to both. Two segments
 * that share an end do not cross, and neither do two that only touch, because a triangulation is
 * allowed both.
 * @param tri the triangulation the points belong to
 * @param a0 one end of the first segment
 * @param a1 the other end of the first segment
 * @param b0 one end of the second segment
 * @param b1 the other end of the second segment
 * @return whether the two segments properly cross
 */
bool
properlyCrosses(const IncrementalDelaunay & tri,
                const std::size_t a0,
                const std::size_t a1,
                const std::size_t b0,
                const std::size_t b1)
{
  if (a0 == b0 || a0 == b1 || a1 == b0 || a1 == b1)
    return false;

  return orientationSign(tri, a0, a1, b0) * orientationSign(tri, a0, a1, b1) < 0 &&
         orientationSign(tri, b0, b1, a0) * orientationSign(tri, b0, b1, a1) < 0;
}

/**
 * Whether any triangle has an edge between two points, in either direction around the triangle.
 * @param triangles the triangles getTriangles() returned
 * @param v0 one end of the edge
 * @param v1 the other end of the edge
 * @return whether the edge is there
 */
bool
hasEdge(const std::vector<Triangle> & triangles, const std::size_t v0, const std::size_t v1)
{
  for (const auto & triangle : triangles)
    for (const auto i : make_range(3u))
    {
      const auto first = triangle.vertices[i];
      const auto second = triangle.vertices[(i + 1) % 3];
      if ((first == v0 && second == v1) || (first == v1 && second == v0))
        return true;
    }

  return false;
}

/**
 * The triangles getTriangles() handed back whose vertices do not run counter-clockwise, which is
 * the order the class promises and the order every consumer of the triangles reads them in.
 * @param tri the triangulation
 * @return one entry describing each such triangle, empty if there are none
 */
std::vector<std::string>
nonCounterClockwiseTriangles(const IncrementalDelaunay & tri)
{
  std::vector<std::string> violations;

  const auto triangles = tri.getTriangles();
  for (const auto t : index_range(triangles))
  {
    const auto & vertices = triangles[t].vertices;
    if (orientationSign(tri, vertices[0], vertices[1], vertices[2]) <= 0)
      violations.push_back("triangle " + std::to_string(t) + " on points " +
                           std::to_string(vertices[0]) + ", " + std::to_string(vertices[1]) +
                           " and " + std::to_string(vertices[2]) +
                           " does not run counter-clockwise");
  }

  return violations;
}

/**
 * Everything wrong with the constrained segments: one that is not reported as constrained when its
 * ends are given the other way round, one that is not an edge of the triangulation, and one that
 * an edge cuts through. checkInvariants() covers the middle of those; the crossing test is here
 * because a segment can be an edge and still be cut somewhere else along its length, which is the
 * shape a flip that ignored the constraint would leave behind.
 * @param tri the triangulation
 * @return one entry describing each violation found, empty if there are none
 */
std::vector<std::string>
constraintViolations(const IncrementalDelaunay & tri)
{
  std::vector<std::string> violations;

  const auto triangles = tri.getTriangles();
  for (const auto & [c0, c1] : tri.constrainedSegments())
  {
    const std::string name =
        "constrained segment (" + std::to_string(c0) + ", " + std::to_string(c1) + ")";

    if (!tri.isConstrainedSegment(c1, c0))
      violations.push_back(name + " is not reported as constrained with its ends the other way "
                                  "round");
    if (!hasEdge(triangles, c0, c1))
      violations.push_back(name + " is not an edge of any triangle");

    for (const auto t : index_range(triangles))
      for (const auto i : make_range(3u))
      {
        const auto v0 = triangles[t].vertices[i];
        const auto v1 = triangles[t].vertices[(i + 1) % 3];
        if (properlyCrosses(tri, v0, v1, c0, c1))
          violations.push_back("the edge of triangle " + std::to_string(t) + " between points " +
                               std::to_string(v0) + " and " + std::to_string(v1) + " crosses " +
                               name);
      }
  }

  return violations;
}

/**
 * The number of triangles a triangulation of n points has, h of them on the boundary of their
 * convex hull. Euler's formula gives V - E + F = 2 with V = n and F the triangles plus the outer
 * face, and 3 * triangles = 2 * E - h because the h boundary edges belong to one triangle each and
 * every other edge to two, which together leave triangles = 2 * n - h - 2.
 * @param n the number of points
 * @param h the number of them on the boundary of the convex hull, collinear ones included
 * @return the number of triangles covering the hull
 */
std::size_t
eulerTriangleCount(const std::size_t n, const std::size_t h)
{
  return 2 * n - h - 2;
}

/**
 * A 4 x 4 grid of points one apart, the classic degenerate input: the four corners of every cell
 * are exactly cocircular, so moose_incircle() comes out exactly zero there and which diagonal the
 * cell gets is a tie the algorithm has to break rather than compute. Both rows and columns are four
 * exactly collinear points, and so are the two long diagonals.
 * @return the sixteen grid points, row by row
 */
std::vector<Point2D>
gridPoints()
{
  return {{0.0, 0.0},
          {1.0, 0.0},
          {2.0, 0.0},
          {3.0, 0.0},
          {0.0, 1.0},
          {1.0, 1.0},
          {2.0, 1.0},
          {3.0, 1.0},
          {0.0, 2.0},
          {1.0, 2.0},
          {2.0, 2.0},
          {3.0, 2.0},
          {0.0, 3.0},
          {1.0, 3.0},
          {2.0, 3.0},
          {3.0, 3.0}};
}

/**
 * A table whose convex hull is the triangle (0, 0), (6, 0), (3, 3) with two more points sitting
 * exactly on its base, so that four of the points are exactly collinear and two of those are on the
 * hull boundary without being corners of it. Point (2, 1) is the only one strictly inside.
 * @return the six points
 */
std::vector<Point2D>
collinearPoints()
{
  return {{0.0, 0.0}, {2.0, 0.0}, {4.0, 0.0}, {6.0, 0.0}, {3.0, 3.0}, {2.0, 1.0}};
}

/**
 * A quadrilateral flat enough that an unconstrained triangulation is certain to split it the other
 * way: points 0 and 1 are 6 apart and points 2 and 3 only 2, so the Delaunay diagonal is (2, 3).
 * Constraining (0, 1) is therefore a constraint the algorithm has to hold in place against the
 * criterion it otherwise follows.
 * @return the four points
 */
std::vector<Point2D>
flatQuadPoints()
{
  return {{0.0, 0.0}, {6.0, 0.0}, {3.0, 1.0}, {3.0, -1.0}};
}

/**
 * Builds the triangulation of a table by inserting the points one at a time, checking after every
 * single insertion that nothing is broken. Checking here rather than only at the end is what
 * catches a corruption that a later insertion would repair, and the point index in the message says
 * which insertion broke it.
 *
 * No table this runs on carries a constrained segment, so the empty circumcircle test is the whole
 * Delaunay contract here rather than a criterion that is too strong.
 * @param points the points to triangulate, all distinct, the first of which initializes the
 * triangulation
 * @param tri filled with the triangulation of the points
 */
void
insertOneAtATime(const std::vector<Point2D> & points, IncrementalDelaunay & tri)
{
  // Starting from the first point on its own is what puts every one of the others in through
  // insertPoint(), where it can be checked by itself.
  tri.initialize({points.front()}, {});

  for (const auto i : make_range(std::size_t(1), points.size()))
  {
    ASSERT_EQ(i, tri.insertPoint(points[i]));
    ASSERT_EQ(i + 1, tri.numPoints());

    const auto after = " after inserting point " + std::to_string(i);
    EXPECT_TRUE(noViolations("checkEmptyCircumcircle()" + after, tri.checkEmptyCircumcircle()));
    EXPECT_TRUE(noViolations("checkInvariants()" + after, tri.checkInvariants()));
    EXPECT_TRUE(noViolations("the triangles getTriangles() returned" + after,
                             nonCounterClockwiseTriangles(tri)));
  }
}

} // namespace

/**
 * A point identical to one already there is not a second vertex: insertPoint() hands back the
 * vertex it already is, the point set does not grow, and the triangulation is left as it was. The
 * coordinates every vertex reports are the ones it went in with.
 */
TEST(IncrementalDelaunayTest, duplicatePointReturnsExistingVertex)
{
  Moose::initPredicates();

  const auto points = collinearPoints();

  IncrementalDelaunay tri;
  tri.initialize(points, {});
  ASSERT_EQ(points.size(), tri.numPoints());

  // The coordinates are held rather than recomputed, so they come back exactly.
  for (const auto i : index_range(points))
  {
    EXPECT_EQ(points[i].x, tri.point(i).x) << "point " << i << " lost its x coordinate";
    EXPECT_EQ(points[i].y, tri.point(i).y) << "point " << i << " lost its y coordinate";
  }

  const auto triangle_count = tri.getTriangles().size();
  for (const auto i : index_range(points))
  {
    EXPECT_EQ(i, tri.insertPoint(points[i])) << "re-inserting point " << i << " made a new vertex";
    EXPECT_EQ(points.size(), tri.numPoints()) << "re-inserting point " << i << " grew the points";
    EXPECT_EQ(triangle_count, tri.getTriangles().size())
        << "re-inserting point " << i << " changed the triangulation";
  }

  EXPECT_TRUE(noViolations("checkInvariants()", tri.checkInvariants()));
}

/**
 * A regular grid is the degenerate case the empty circumcircle test is worth running on, because
 * every cell of it is a square whose four corners are exactly cocircular. The triangulation stays
 * Delaunay through every one of the fifteen insertions and not only at the end of them.
 */
TEST(IncrementalDelaunayTest, regularGridStaysDelaunayAfterEveryInsertion)
{
  Moose::initPredicates();

  const auto points = gridPoints();

  IncrementalDelaunay tri;
  insertOneAtATime(points, tri);
  ASSERT_EQ(points.size(), tri.numPoints());

  // Every point is a vertex of the result, which a tie broken the wrong way could otherwise drop.
  std::set<std::size_t> used;
  for (const auto & triangle : tri.getTriangles())
    used.insert(triangle.vertices.begin(), triangle.vertices.end());

  for (const auto i : index_range(points))
    EXPECT_TRUE(used.count(i) > 0) << "point " << i << " is not a vertex of any triangle";
}

/**
 * Four of these points are exactly collinear and two of those are on the boundary of the convex
 * hull without being corners of it, so the base of the hull has to come out as the three short
 * edges between consecutive points rather than as the one long edge between its ends.
 */
TEST(IncrementalDelaunayTest, collinearPointsStayDelaunayAfterEveryInsertion)
{
  Moose::initPredicates();

  const auto points = collinearPoints();

  IncrementalDelaunay tri;
  insertOneAtATime(points, tri);
  ASSERT_EQ(points.size(), tri.numPoints());

  const auto triangles = tri.getTriangles();
  EXPECT_TRUE(hasEdge(triangles, 0, 1));
  EXPECT_TRUE(hasEdge(triangles, 1, 2));
  EXPECT_TRUE(hasEdge(triangles, 2, 3));

  // An edge from one end of the base to the other would run straight through points 1 and 2.
  EXPECT_FALSE(hasEdge(triangles, 0, 3));
}

/**
 * A constrained segment stays an edge through every later insertion, and nothing crosses it.
 *
 * The segment is the long diagonal of a flat quadrilateral, which is where the constraint logic
 * earns its keep: the circumcircle of triangle (0, 1, 2) is centered at (3, -4) with a radius of 5
 * and holds point 3, which is 3 away from that center, so an unconstrained triangulation would
 * flip (0, 1) to (2, 3) instead. checkEmptyCircumcircle() therefore has to report a violation here,
 * while checkInvariants(), which exempts constrained edges from that test, must not.
 */
TEST(IncrementalDelaunayTest, constrainedSegmentSurvivesInsertions)
{
  Moose::initPredicates();

  const auto points = flatQuadPoints();

  IncrementalDelaunay tri;
  tri.initialize(points, {Segment(0, 1)});

  EXPECT_TRUE(tri.isConstrainedSegment(0, 1));
  EXPECT_TRUE(tri.isConstrainedSegment(1, 0));
  EXPECT_EQ(std::set<Segment>({Segment(0, 1)}), tri.constrainedSegments());

  const auto triangles = tri.getTriangles();
  EXPECT_TRUE(hasEdge(triangles, 0, 1)) << "the constrained diagonal is not an edge";
  EXPECT_FALSE(hasEdge(triangles, 2, 3)) << "the diagonal the constraint rules out is an edge";
  EXPECT_FALSE(tri.checkEmptyCircumcircle().empty())
      << "the constrained edge is locally Delaunay after all, so this configuration does not test "
         "the constraint";

  EXPECT_TRUE(noViolations("checkInvariants()", tri.checkInvariants()));
  EXPECT_TRUE(noViolations("the constrained segments", constraintViolations(tri)));

  // Points strictly inside one half or the other, none of them on the segment itself.
  const std::vector<Point2D> inserted = {{3.0, 0.5}, {2.0, 0.25}, {3.0, -0.5}, {4.0, -0.25}};
  for (const auto i : index_range(inserted))
  {
    const auto id = tri.insertPoint(inserted[i]);
    ASSERT_EQ(points.size() + i, id);

    const auto after = " after inserting point " + std::to_string(id);
    EXPECT_TRUE(tri.isConstrainedSegment(0, 1)) << "the segment stopped being constrained" << after;
    EXPECT_TRUE(noViolations("checkInvariants()" + after, tri.checkInvariants()));
    EXPECT_TRUE(noViolations("the constrained segments" + after, constraintViolations(tri)));
    EXPECT_TRUE(noViolations("the triangles getTriangles() returned" + after,
                             nonCounterClockwiseTriangles(tri)));
  }

  EXPECT_EQ(std::set<Segment>({Segment(0, 1)}), tri.constrainedSegments());
  EXPECT_TRUE(hasEdge(tri.getTriangles(), 0, 1));
}

/**
 * A point that lands exactly on a constrained segment divides it, because a segment with a vertex
 * in the middle of it can no longer be a single edge. The two halves are constrained in its place,
 * each recorded with the smaller vertex id first.
 */
TEST(IncrementalDelaunayTest, pointOnConstrainedSegmentSplitsIt)
{
  Moose::initPredicates();

  const auto points = flatQuadPoints();

  IncrementalDelaunay tri;
  tri.initialize(points, {Segment(0, 1)});
  ASSERT_TRUE(tri.isConstrainedSegment(0, 1));

  // (2, 0) is exactly on the segment from (0, 0) to (6, 0).
  const auto split = tri.insertPoint({2.0, 0.0});
  ASSERT_EQ(points.size(), split);
  ASSERT_EQ(points.size() + 1, tri.numPoints());

  EXPECT_FALSE(tri.isConstrainedSegment(0, 1)) << "the divided segment is still constrained whole";
  EXPECT_TRUE(tri.isConstrainedSegment(0, split));
  EXPECT_TRUE(tri.isConstrainedSegment(split, 1));
  EXPECT_EQ(std::set<Segment>({Segment(0, split), Segment(1, split)}), tri.constrainedSegments());

  EXPECT_TRUE(noViolations("checkInvariants()", tri.checkInvariants()));
  EXPECT_TRUE(noViolations("the constrained segments", constraintViolations(tri)));
  EXPECT_TRUE(
      noViolations("the triangles getTriangles() returned", nonCounterClockwiseTriangles(tri)));
}

/**
 * getTriangles() drops the triangles that touch the bounding triangle, so what it hands back covers
 * exactly the convex hull of the points and Euler's formula fixes how many of them there are. Every
 * count below is the formula and a second, independent reading of the same configuration.
 */
TEST(IncrementalDelaunayTest, triangleCountMatchesEulerFormula)
{
  Moose::initPredicates();

  // The 4 x 4 grid: its hull is the square from (0, 0) to (3, 3), whose boundary carries the twelve
  // points of the perimeter, leaving the four in the middle inside. 2 * 16 - 12 - 2 = 18, which is
  // also the nine cells of the grid cut into two triangles each.
  IncrementalDelaunay grid;
  grid.initialize(gridPoints(), {});
  EXPECT_EQ(eulerTriangleCount(16, 12), grid.getTriangles().size());

  // The collinear table: its hull is the triangle (0, 0), (6, 0), (3, 3), and points 1 and 2 count
  // towards h even though they are not corners of it, because they are on its boundary. So h is 5
  // rather than 3, and 2 * 6 - 5 - 2 = 5. Reading it the other way, a region with 5 boundary
  // vertices and 1 point inside takes 5 + 2 * 1 - 2 = 5 triangles.
  IncrementalDelaunay collinear;
  collinear.initialize(collinearPoints(), {});
  EXPECT_EQ(eulerTriangleCount(6, 5), collinear.getTriangles().size());

  // A square with its center: the four corners are the hull and the center is inside, so
  // 2 * 5 - 4 - 2 = 4, the fan of four triangles around the center.
  IncrementalDelaunay fan;
  fan.initialize({{0.0, 0.0}, {4.0, 0.0}, {4.0, 4.0}, {0.0, 4.0}, {2.0, 2.0}}, {});
  EXPECT_EQ(eulerTriangleCount(5, 4), fan.getTriangles().size());

  // A constrained segment changes which triangles cover the hull but not how many, so the count is
  // the same 2 * 4 - 4 - 2 = 2 either diagonal splits the quadrilateral into.
  IncrementalDelaunay constrained;
  constrained.initialize(flatQuadPoints(), {Segment(0, 1)});
  EXPECT_EQ(eulerTriangleCount(4, 4), constrained.getTriangles().size());
}

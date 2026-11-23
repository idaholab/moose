//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PolygonUtils.h"

using namespace libMesh;

TEST(PolygonUtilsTest, addUniquePoint)
{
  std::vector<Point> points;

  polygon_utils::addUniquePoint(points, Point(0, 0, 0), 1e-12);
  polygon_utils::addUniquePoint(points, Point(1, 0, 0), 1e-12);

  // Within the tolerance of a point already there
  polygon_utils::addUniquePoint(points, Point(1e-13, 0, 0), 1e-12);
  EXPECT_EQ(points.size(), 2u);

  // Outside it
  polygon_utils::addUniquePoint(points, Point(1e-11, 0, 0), 1e-12);
  EXPECT_EQ(points.size(), 3u);
}

TEST(PolygonUtilsTest, pointOnSegment2D)
{
  const Point a(0, 0, 0);
  const Point b(2, 0, 0);

  EXPECT_TRUE(polygon_utils::pointOnSegment2D(Point(1, 0, 0), a, b));
  EXPECT_TRUE(polygon_utils::pointOnSegment2D(a, a, b));
  EXPECT_TRUE(polygon_utils::pointOnSegment2D(b, a, b));

  // Off the line, and on the line but past the end
  EXPECT_FALSE(polygon_utils::pointOnSegment2D(Point(1, 1, 0), a, b));
  EXPECT_FALSE(polygon_utils::pointOnSegment2D(Point(3, 0, 0), a, b));

  // A degenerate segment is the single point it collapses to
  EXPECT_TRUE(polygon_utils::pointOnSegment2D(a, a, a));
  EXPECT_FALSE(polygon_utils::pointOnSegment2D(b, a, a));
}

TEST(PolygonUtilsTest, pointInPolygon2D)
{
  // An L shape, so that a convex half-space test would get this wrong
  const std::vector<Point> concave = {Point(0, 0, 0),
                                      Point(2, 0, 0),
                                      Point(2, 1, 0),
                                      Point(1, 1, 0),
                                      Point(1, 2, 0),
                                      Point(0, 2, 0)};

  EXPECT_TRUE(polygon_utils::pointInPolygon2D(Point(0.5, 0.5, 0), concave));
  EXPECT_TRUE(polygon_utils::pointInPolygon2D(Point(1.5, 0.5, 0), concave));
  EXPECT_TRUE(polygon_utils::pointInPolygon2D(Point(0.5, 1.5, 0), concave));

  // The notch of the L is outside, even though it is inside the convex hull
  EXPECT_FALSE(polygon_utils::pointInPolygon2D(Point(1.5, 1.5, 0), concave));
  EXPECT_FALSE(polygon_utils::pointInPolygon2D(Point(3, 3, 0), concave));

  // A point on the boundary counts as inside
  EXPECT_TRUE(polygon_utils::pointInPolygon2D(Point(1, 0, 0), concave));
  EXPECT_TRUE(polygon_utils::pointInPolygon2D(Point(0, 0, 0), concave));

  // Fewer than three corners is not a polygon
  EXPECT_FALSE(polygon_utils::pointInPolygon2D(Point(0, 0, 0),
                                               {Point(0, 0, 0), Point(1, 0, 0)}));
}

TEST(PolygonUtilsTest, addSegmentIntersections2D)
{
  const Real tol = 1e-12;

  // Crossing segments meet at one point
  std::vector<Point> points;
  polygon_utils::addSegmentIntersections2D(
      points, Point(0, 0, 0), Point(2, 0, 0), Point(1, -1, 0), Point(1, 1, 0), tol);
  ASSERT_EQ(points.size(), 1u);
  EXPECT_NEAR(points[0](0), 1.0, tol);
  EXPECT_NEAR(points[0](1), 0.0, tol);

  // Disjoint segments do not
  points.clear();
  polygon_utils::addSegmentIntersections2D(
      points, Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0), Point(1, 1, 0), tol);
  EXPECT_TRUE(points.empty());

  // Overlapping collinear segments give the end points of the overlap
  points.clear();
  polygon_utils::addSegmentIntersections2D(
      points, Point(0, 0, 0), Point(2, 0, 0), Point(1, 0, 0), Point(3, 0, 0), tol);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_NEAR(points[0](0), 1.0, tol);
  EXPECT_NEAR(points[1](0), 2.0, tol);

  // A degenerate segment contributes its point when it lies on the other segment
  points.clear();
  polygon_utils::addSegmentIntersections2D(
      points, Point(1, 0, 0), Point(1, 0, 0), Point(0, 0, 0), Point(2, 0, 0), tol);
  ASSERT_EQ(points.size(), 1u);
  EXPECT_NEAR(points[0](0), 1.0, tol);
}

TEST(PolygonUtilsTest, fanApexOffset)
{
  const Real area_tol = 1e-12;

  // A square fans soundly from any vertex
  const std::vector<Point> square = {
      Point(0, 0, 0), Point(1, 0, 0), Point(1, 1, 0), Point(0, 1, 0)};
  EXPECT_EQ(polygon_utils::fanApexOffset(square, area_tol), 0u);

  // With a mid-side node on the bottom, vertex 0 is collinear with the edge from the node to the
  // next corner, so the fan from it is degenerate and the apex has to move
  const std::vector<Point> with_midpoint = {Point(0, 0, 0),
                                            Point(0.5, 0, 0),
                                            Point(1, 0, 0),
                                            Point(1, 1, 0),
                                            Point(0, 1, 0)};
  const auto apex = polygon_utils::fanApexOffset(with_midpoint, area_tol);
  EXPECT_NE(apex, 0u);

  // The returned apex must really fan into non-degenerate triangles
  const auto n = with_midpoint.size();
  for (const auto i : make_range(std::size_t(1), n - 1))
  {
    const Point & a = with_midpoint[apex];
    const Point & b = with_midpoint[(apex + i) % n];
    const Point & c = with_midpoint[(apex + i + 1) % n];
    const Real twice_area = (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0));
    EXPECT_GT(std::abs(twice_area), area_tol);
  }
}

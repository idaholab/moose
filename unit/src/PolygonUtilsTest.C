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
  EXPECT_FALSE(polygon_utils::pointInPolygon2D(Point(0, 0, 0), {Point(0, 0, 0), Point(1, 0, 0)}));
}

TEST(PolygonUtilsTest, fanApexOffset)
{
  const Real area_tol = 1e-12;

  // A square fans soundly from any corner
  const std::vector<Point> square = {
      Point(0, 0, 0), Point(1, 0, 0), Point(1, 1, 0), Point(0, 1, 0)};
  EXPECT_EQ(polygon_utils::fanApexOffset(square, area_tol), 0u);

  // With a mid-side node on the bottom, corner 0 is an end of the collinear run along that side,
  // so the fan from it spans the rest of the run as a zero-area triangle and the apex has to move.
  // The polygon itself is valid; only this apex is not.
  const std::vector<Point> with_midpoint = {
      Point(0, 0, 0), Point(0.5, 0, 0), Point(1, 0, 0), Point(1, 1, 0), Point(0, 1, 0)};
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

  // A corner interior to the collinear run is a sound apex: both run edges are adjacent to it, so
  // the fan never spans the run
  EXPECT_EQ(polygon_utils::fanApexOffset(
                {Point(0.5, 0, 0), Point(1, 0, 0), Point(1, 1, 0), Point(0, 1, 0), Point(0, 0, 0)},
                area_tol),
            0u);
}

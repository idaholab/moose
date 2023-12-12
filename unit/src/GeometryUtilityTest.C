//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometryUtilityTest.h"

TEST_F(GeometryUtilityTest, norm)
{
  Point n(0.0, 0.0, 0.0);
  Point pt;
  try
  {
    pt = geom_utils::unitVector(n, "n");
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("'n' cannot have zero norm!"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  Point m(5.0, 0.0, 0.0);
  pt = geom_utils::unitVector(m, "m");
  EXPECT_DOUBLE_EQ(pt(0), 1.0);
  EXPECT_DOUBLE_EQ(pt(1), 0.0);
  EXPECT_DOUBLE_EQ(pt(2), 0.0);
}

TEST_F(GeometryUtilityTest, rotate_about_y)
{
  Point n(0.0, 0.0, -1.0);
  Point a(0.0, 1.0, 0.0);

  Point pt(0.0, 0.0, -1.0);
  Point ptr = geom_utils::rotatePointAboutAxis(pt, M_PI / 4.0, a);
  EXPECT_DOUBLE_EQ(ptr(0), -sqrt(2.0) / 2.0);
  EXPECT_DOUBLE_EQ(ptr(1), 0.0);
  EXPECT_DOUBLE_EQ(ptr(2), -sqrt(2.0) / 2.0);
}

TEST_F(GeometryUtilityTest, rotate_about_x)
{
  Point n(0.0, 1.0, 0.0);
  Point a(0.5, 0.0, 0.0);
  a = a / a.norm();

  // already on x-axis, shouldn't move
  Point pt(1.0, 0.0, 0.0);
  Point ptr = geom_utils::rotatePointAboutAxis(pt, M_PI / 2.0, a);
  EXPECT_DOUBLE_EQ(ptr(0), 1.0);
  EXPECT_DOUBLE_EQ(ptr(1), 0.0);
  EXPECT_DOUBLE_EQ(ptr(2), 0.0);

  pt = {0.0, 1.0, 0.0};
  ptr = geom_utils::rotatePointAboutAxis(pt, 3.0 * M_PI / 4.0, a);
  EXPECT_DOUBLE_EQ(ptr(0), 0.0);
  EXPECT_DOUBLE_EQ(ptr(1), -sqrt(2.0) / 2.0);
  EXPECT_DOUBLE_EQ(ptr(2), sqrt(2.0) / 2.0);
}

TEST_F(GeometryUtilityTest, polygon_corners)
{
  // 4-sided polygon
  auto c = geom_utils::polygonCorners(4, 1.0, 2);
  Real side_length = std::sqrt(0.5);

  EXPECT_DOUBLE_EQ(c[0](0), side_length);
  EXPECT_DOUBLE_EQ(c[0](1), side_length);
  EXPECT_DOUBLE_EQ(c[0](2), 0.0);

  EXPECT_DOUBLE_EQ(c[1](0), -side_length);
  EXPECT_DOUBLE_EQ(c[1](1), side_length);
  EXPECT_DOUBLE_EQ(c[1](2), 0.0);

  EXPECT_DOUBLE_EQ(c[2](0), -side_length);
  EXPECT_DOUBLE_EQ(c[2](1), -side_length);
  EXPECT_DOUBLE_EQ(c[2](2), 0.0);

  EXPECT_DOUBLE_EQ(c[3](0), side_length);
  EXPECT_DOUBLE_EQ(c[3](1), -side_length);
  EXPECT_DOUBLE_EQ(c[3](2), 0.0);

  // 6-sided polygon
  c = geom_utils::polygonCorners(6, 1.0, 2);
  const Real COS60 = 0.5;
  const Real SIN60 = std::sqrt(3.0) / 2.0;
  Real tol = 1e-6;
  side_length = 1.0;

  EXPECT_NEAR(c[0](0), side_length * COS60, tol);
  EXPECT_NEAR(c[0](1), side_length * SIN60, tol);
  EXPECT_NEAR(c[0](2), 0.0, tol);

  EXPECT_NEAR(c[1](0), -side_length * COS60, tol);
  EXPECT_NEAR(c[1](1), side_length * SIN60, tol);
  EXPECT_NEAR(c[1](2), 0.0, tol);

  EXPECT_NEAR(c[2](0), -side_length, tol);
  EXPECT_NEAR(c[2](1), 0.0, tol);
  EXPECT_NEAR(c[2](2), 0.0, tol);

  EXPECT_NEAR(c[3](0), -side_length * COS60, tol);
  EXPECT_NEAR(c[3](1), -side_length * SIN60, tol);
  EXPECT_NEAR(c[3](2), 0.0, tol);

  EXPECT_NEAR(c[4](0), side_length * COS60, tol);
  EXPECT_NEAR(c[4](1), -side_length * SIN60, tol);
  EXPECT_NEAR(c[4](2), 0.0, tol);

  EXPECT_NEAR(c[5](0), side_length, tol);
  EXPECT_NEAR(c[5](1), 0.0, tol);
  EXPECT_NEAR(c[5](2), 0.0, tol);
}

TEST_F(GeometryUtilityTest, line_half_space)
{
  Point p1(1.0, 1.0, 1.0);
  Point p2(2.0, 2.0, 5.0);

  Point p(2.0, 1.0, 9.0);
  EXPECT_TRUE(geom_utils::projectedLineHalfSpace(p, p1, p2, 2) < 0.0);

  Point q(-1.0, 0.0, -3.0);
  EXPECT_TRUE(geom_utils::projectedLineHalfSpace(q, p1, p2, 2) > 0.0);
}

TEST_F(GeometryUtilityTest, projected_unit_normal)
{
  Point p1(1.0, 1.0, 1.0);
  Point p2(2.0, 2.0, 5.0);
  auto n1 = geom_utils::projectedUnitNormal(p1, p2, 2);
  EXPECT_DOUBLE_EQ(n1(0), -std::sqrt(2.0) / 2.0);
  EXPECT_DOUBLE_EQ(n1(1), std::sqrt(2.0) / 2.0);
  EXPECT_DOUBLE_EQ(n1(2), 0.0);
}

TEST_F(GeometryUtilityTest, projected_line_distance)
{
  // horizontal line
  Point p1(4.0, 5.0, 4.0);
  Point l1(1.0, 3.0, 3.0);
  Point l2(5.0, 3.0, 6.0);
  EXPECT_DOUBLE_EQ(geom_utils::projectedDistanceFromLine(p1, l1, l2, 2), 2.0);

  // vertical line
  Point l4(1.0, 5.0, 4.0);
  Point l3(1.0, 3.0, 3.0);
  Point p2(3.0, 4.0, 6.0);
  EXPECT_DOUBLE_EQ(geom_utils::projectedDistanceFromLine(p2, l3, l4, 2), 2.0);

  // angled line
  Point p3(2.0, 2.0, 4.0);
  Point l5(1.0, 2.0, 3.0);
  Point l6(2.0, 3.0, 6.0);
  EXPECT_DOUBLE_EQ(geom_utils::projectedDistanceFromLine(p3, l5, l6, 2), std::sqrt(2.0) / 2.0);
}

TEST_F(GeometryUtilityTest, line_distance)
{
  // horizontal line
  Point p1(4.0, 5.0, 0.0);
  Point l1(1.0, 3.0, 0.0);
  Point l2(5.0, 3.0, 0.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p1, l1, l2), 2.0);

  // vertical line
  Point l4(1.0, 5.0, 0.0);
  Point l3(1.0, 3.0, 0.0);
  Point p2(3.0, 4.0, 0.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p2, l3, l4), 2.0);

  // angled line
  Point p3(2.0, 2.0, 0.0);
  Point l5(1.0, 2.0, 0.0);
  Point l6(2.0, 3.0, 0.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p3, l5, l6), std::sqrt(2.0) / 2.0);

  // now place the lines in the y-z plane
  Point p4(0.0, 4.0, 5.0);
  Point l7(0.0, 1.0, 3.0);
  Point l8(0.0, 5.0, 3.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p4, l7, l8), 2.0);

  // vertical line
  Point l10(0.0, 1.0, 5.0);
  Point l9(0.0, 1.0, 3.0);
  Point p5(0.0, 3.0, 4.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p5, l9, l10), 2.0);

  // angled line
  Point p6(0.0, 2.0, 2.0);
  Point l11(0.0, 1.0, 2.0);
  Point l12(0.0, 2.0, 3.0);
  EXPECT_DOUBLE_EQ(geom_utils::distanceFromLine(p6, l11, l12), std::sqrt(2.0) / 2.0);
}

TEST_F(GeometryUtilityTest, point_on_edge)
{
  // test points on a simpler Cartesian-based polygon
  Point pt8(1.0, 2.0, 0.0);
  Point pt9(2.0, 3.0, 0.0);
  Point pt10(3.0, 3.0, 0.0);
  Point pt11(3.0, 1.0, 0.0);

  Point pt0(3.0, 1.0, 0.0);
  Point pt1(3.0, 1.1, 0.0);
  Point pt2(3.1, 1.0, 0.0);
  Point pt3(3.0, 0.9, 0.0);

  EXPECT_TRUE(geom_utils::pointInPolygon(pt0, {pt8, pt9, pt10, pt11}, 2));
  EXPECT_TRUE(geom_utils::pointInPolygon(pt1, {pt8, pt9, pt10, pt11}, 2));
  EXPECT_FALSE(geom_utils::pointInPolygon(pt2, {pt8, pt9, pt10, pt11}, 2));
  EXPECT_FALSE(geom_utils::pointInPolygon(pt3, {pt8, pt9, pt10, pt11}, 2));
}

TEST_F(GeometryUtilityTest, point_in_polygon)
{
  // triangle
  Point pt1(1.0, 1.0, 0.0);
  Point pt2(3.0, 2.0, 0.0);
  Point pt3(2.0, 2.0, 0.0);

  Point pt_in(2.0, 1.9, 0.0);
  Point pt_not_in(2.0, 3.0, 0.0);
  Point pt_edge = pt1;

  EXPECT_TRUE(geom_utils::pointInPolygon(pt_in, {pt1, pt2, pt3}, 2));
  EXPECT_FALSE(geom_utils::pointInPolygon(pt_not_in, {pt1, pt2, pt3}, 2));
  EXPECT_TRUE(geom_utils::pointInPolygon(pt_edge, {pt1, pt2, pt3}, 2));

  // rectangle
  Point pt4(1.0, 2.0, 0.0);
  Point pt5(2.0, 1.0, 0.0);
  Point pt6(4.0, 3.0, 0.0);
  Point pt7(3.0, 4.0, 0.0);

  Point pt_in1(2.0, 2.0, 0.0);
  Point pt_not_in1(3.0, 1.0, 0.0);
  pt_edge = pt5;

  EXPECT_TRUE(geom_utils::pointInPolygon(pt_in1, {pt4, pt5, pt6, pt7}, 2));
  EXPECT_FALSE(geom_utils::pointInPolygon(pt_not_in1, {pt4, pt5, pt6, pt7}, 2));
  EXPECT_TRUE(geom_utils::pointInPolygon(pt_edge, {pt4, pt5, pt6, pt7}, 2));

  // general polygon
  Point pt8(1.0, 2.0, 0.0);
  Point pt9(2.0, 3.0, 0.0);
  Point pt10(3.0, 3.0, 0.0);
  Point pt11(3.0, 1.0, 0.0);

  Point pt_in2(2.0, 2.0, 0.0);
  Point pt_not_in2(1.0, 3.0, 0.0);
  Point pt_edge2(3.0, 2.0, 0.0);

  EXPECT_TRUE(geom_utils::pointInPolygon(pt_in2, {pt8, pt9, pt10, pt11}, 2));
  EXPECT_FALSE(geom_utils::pointInPolygon(pt_not_in2, {pt8, pt9, pt10, pt11}, 2));
  EXPECT_TRUE(geom_utils::pointInPolygon(pt_edge2, {pt8, pt9, pt10, pt11}, 2));
}

TEST_F(GeometryUtilityTest, boxCorners)
{
  Point min(-1.0, -0.5, 4.0);
  Point max(3.0, 0.5, 6.0);

  BoundingBox box(min, max);
  auto c = geom_utils::boxCorners(box, 0.85);
  EXPECT_DOUBLE_EQ(c[0](0), -0.7);
  EXPECT_DOUBLE_EQ(c[0](1), -0.425);
  EXPECT_DOUBLE_EQ(c[0](2), 4.15);

  EXPECT_DOUBLE_EQ(c[1](0), 2.7);
  EXPECT_DOUBLE_EQ(c[1](1), -0.425);
  EXPECT_DOUBLE_EQ(c[1](2), 4.15);

  EXPECT_DOUBLE_EQ(c[2](0), -0.7);
  EXPECT_DOUBLE_EQ(c[2](1), 0.425);
  EXPECT_DOUBLE_EQ(c[2](2), 4.15);

  EXPECT_DOUBLE_EQ(c[3](0), 2.7);
  EXPECT_DOUBLE_EQ(c[3](1), 0.425);
  EXPECT_DOUBLE_EQ(c[3](2), 4.15);

  EXPECT_DOUBLE_EQ(c[4](0), -0.7);
  EXPECT_DOUBLE_EQ(c[4](1), -0.425);
  EXPECT_DOUBLE_EQ(c[4](2), 5.85);

  EXPECT_DOUBLE_EQ(c[5](0), 2.7);
  EXPECT_DOUBLE_EQ(c[5](1), -0.425);
  EXPECT_DOUBLE_EQ(c[5](2), 5.85);

  EXPECT_DOUBLE_EQ(c[6](0), -0.7);
  EXPECT_DOUBLE_EQ(c[6](1), 0.425);
  EXPECT_DOUBLE_EQ(c[6](2), 5.85);

  EXPECT_DOUBLE_EQ(c[7](0), 2.7);
  EXPECT_DOUBLE_EQ(c[7](1), 0.425);
  EXPECT_DOUBLE_EQ(c[7](2), 5.85);
}

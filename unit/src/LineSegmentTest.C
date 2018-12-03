//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegmentTest.h"

#include "libmesh/plane.h"

TEST_F(LineSegmentTest, closestPointTest)
{
  // positive x end cases
  EXPECT_EQ(_posx.closest_point(Point(-1, 0)), Point(0, 0));
  EXPECT_EQ(_posx.closest_point(Point(0, -1)), Point(0, 0));
  EXPECT_EQ(_posx.closest_point(Point(0, 1)), Point(0, 0));
  EXPECT_EQ(_posx.closest_point(Point(6, 0)), Point(5, 0));
  EXPECT_EQ(_posx.closest_point(Point(6, -1)), Point(5, 0));
  EXPECT_EQ(_posx.closest_point(Point(6, 1)), Point(5, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(-1, 0, 0)), Point(0, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(0, -1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(0, 1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(6, 0, 0)), Point(5, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(6, -1, 0)), Point(5, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(6, 1, 0)), Point(5, 0, 0));
  // middle of the line
  EXPECT_EQ(_posx.closest_point(Point(2, 0)), Point(2, 0));
  EXPECT_EQ(_posx.closest_point(Point(2, 100000)), Point(2, 0));
  EXPECT_EQ(_posx.closest_point(Point(4, -100000)), Point(4, 0));
  EXPECT_EQ(_posx.closest_point(Point(0.125, -0.125)), Point(0.125, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(2, 0, 0)), Point(2, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(2, 100000, 10000)), Point(2, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(4, -100000, 10000)), Point(4, 0, 0));
  EXPECT_EQ(_pos3x.closest_point(Point(0.125, -0.125, 0.125)), Point(0.125, 0, 0));

  // positive y end cases
  EXPECT_EQ(_posy.closest_point(Point(0, -1)), Point(0, 0));
  EXPECT_EQ(_posy.closest_point(Point(-1, 0)), Point(0, 0));
  EXPECT_EQ(_posy.closest_point(Point(1, 0)), Point(0, 0));
  EXPECT_EQ(_posy.closest_point(Point(0, 6)), Point(0, 5));
  EXPECT_EQ(_posy.closest_point(Point(-1, 6)), Point(0, 5));
  EXPECT_EQ(_posy.closest_point(Point(1, 6)), Point(0, 5));
  // middle of the line
  EXPECT_EQ(_posy.closest_point(Point(0, 2)), Point(0, 2));
  EXPECT_EQ(_posy.closest_point(Point(100000, 2)), Point(0, 2));
  EXPECT_EQ(_posy.closest_point(Point(-100000, 4)), Point(0, 4));
  EXPECT_EQ(_posy.closest_point(Point(-0.125, 0.125)), Point(0, 0.125));

  // negative y end cases
  EXPECT_EQ(_negy.closest_point(Point(0, 1)), Point(0, 0));
  EXPECT_EQ(_negy.closest_point(Point(-1, 0)), Point(0, 0));
  EXPECT_EQ(_negy.closest_point(Point(1, 0)), Point(0, 0));
  EXPECT_EQ(_negy.closest_point(Point(0, -6)), Point(0, -5));
  EXPECT_EQ(_negy.closest_point(Point(-1, -6)), Point(0, -5));
  EXPECT_EQ(_negy.closest_point(Point(1, -6)), Point(0, -5));
  EXPECT_EQ(_neg3y.closest_point(Point(0, 1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(0, 0, 1)), Point(0, 0, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(-1, 0, 1)), Point(0, 0, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(-1, 0, -1)), Point(0, 0, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(1, 0, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(0, -6, 0)), Point(0, -5, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(-1, -6, -9)), Point(0, -5, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(1, -6, 9)), Point(0, -5, 0));
  // middle of the line
  EXPECT_EQ(_negy.closest_point(Point(0, -2)), Point(0, -2));
  EXPECT_EQ(_negy.closest_point(Point(100000, -2)), Point(0, -2));
  EXPECT_EQ(_negy.closest_point(Point(-100000, -4)), Point(0, -4));
  EXPECT_EQ(_negy.closest_point(Point(-0.125, -0.125)), Point(0, -0.125));
  EXPECT_EQ(_neg3y.closest_point(Point(0, -2, 0)), Point(0, -2, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(100000, -2, -1000)), Point(0, -2, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(-100000, -4, 333)), Point(0, -4, 0));
  EXPECT_EQ(_neg3y.closest_point(Point(-0.125, -0.125, 2)), Point(0, -0.125, 0));

  // positive diagonal end cases
  EXPECT_EQ(_posdiag.closest_point(Point(0, -1)), Point(0, 0));
  EXPECT_EQ(_posdiag.closest_point(Point(-1, -1)), Point(0, 0));
  EXPECT_EQ(_posdiag.closest_point(Point(0, 0)), Point(0, 0));
  EXPECT_EQ(_posdiag.closest_point(Point(6, 6)), Point(5, 5));
  EXPECT_EQ(_posdiag.closest_point(Point(7, 6)), Point(5, 5));
  EXPECT_EQ(_posdiag.closest_point(Point(6, 9)), Point(5, 5));
  EXPECT_EQ(_pos3diag.closest_point(Point(0, -1, 1)), Point(0, 0, 0));
  EXPECT_EQ(_pos3diag.closest_point(Point(-1, -1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_pos3diag.closest_point(Point(0, 0, 0)), Point(0, 0, 0));
  EXPECT_EQ(_pos3diag.closest_point(Point(6, 6, 6)), Point(5, 5, 5));
  EXPECT_EQ(_pos3diag.closest_point(Point(7, 6, 5)), Point(5, 5, 5));
  EXPECT_EQ(_pos3diag.closest_point(Point(6, 9, 8)), Point(5, 5, 5));
  // middle of the line
  EXPECT_EQ(_posdiag.closest_point(Point(2, 2)), Point(2, 2));
  EXPECT_EQ(_posdiag.closest_point(Point(0, 2)), Point(1, 1));
  EXPECT_EQ(_posdiag.closest_point(Point(2, 0)), Point(1, 1));
  EXPECT_EQ(_posdiag.closest_point(Point(1, 3)), Point(2, 2));
  EXPECT_EQ(_pos3diag.closest_point(Point(2, 2, 2)), Point(2, 2, 2));
  EXPECT_EQ(_pos3diag.closest_point(Point(0, 0, 4)), Point(4. / 3., 4. / 3., 4. / 3.));
  EXPECT_EQ(_pos3diag.closest_point(Point(0, 4, 4)), Point(8. / 3., 8. / 3., 8. / 3.));

  // negative diagonal end cases
  EXPECT_EQ(_negdiag.closest_point(Point(0, 1)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(1, 1)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(0, 0)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(-6, -6)), Point(-5, -5));
  EXPECT_EQ(_negdiag.closest_point(Point(-7, -6)), Point(-5, -5));
  EXPECT_EQ(_negdiag.closest_point(Point(-6, -9)), Point(-5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(1, 1, 1)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 0, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(-6, -6, -6)), Point(-5, -5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(-7, -6, -9)), Point(-5, -5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(-6, -9, -5)), Point(-5, -5, -5));
  // middle of the line
  EXPECT_EQ(_negdiag.closest_point(Point(-2, -2)), Point(-2, -2));
  EXPECT_EQ(_negdiag.closest_point(Point(0, -2)), Point(-1, -1));
  EXPECT_EQ(_negdiag.closest_point(Point(-2, 0)), Point(-1, -1));
  EXPECT_EQ(_negdiag.closest_point(Point(-1, -3)), Point(-2, -2));
  EXPECT_EQ(_neg3diag.closest_point(Point(-2, -2, -2)), Point(-2, -2, -2));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 0, -4)), Point(-4. / 3., -4. / 3., -4. / 3.));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, -4, -4)), Point(-8. / 3., -8. / 3., -8. / 3.));
}

TEST_F(LineSegmentTest, closestNormalPointTest)
{
  Point result;
  EXPECT_TRUE(_posx.closest_normal_point(Point(0, 0), result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_posx.closest_normal_point(Point(0, 10), result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_posx.closest_normal_point(Point(5, -1), result));
  EXPECT_EQ(result, Point(5, 0));
  EXPECT_TRUE(_posx.closest_normal_point(Point(2, 2), result));
  EXPECT_EQ(result, Point(2, 0));
  EXPECT_FALSE(_posx.closest_normal_point(Point(6, -1), result));
  EXPECT_EQ(result, Point(6, 0));
  EXPECT_FALSE(_posx.closest_normal_point(Point(-9, 2), result));
  EXPECT_EQ(result, Point(-9, 0));
  EXPECT_TRUE(_pos3x.closest_normal_point(Point(0, 0, 0), result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3x.closest_normal_point(Point(0, 10, -5), result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3x.closest_normal_point(Point(5, -1, 1), result));
  EXPECT_EQ(result, Point(5, 0, 0));
  EXPECT_TRUE(_pos3x.closest_normal_point(Point(2, 2, 2), result));
  EXPECT_EQ(result, Point(2, 0, 0));
  EXPECT_FALSE(_pos3x.closest_normal_point(Point(6, -1, 0), result));
  EXPECT_EQ(result, Point(6, 0, 0));
  EXPECT_FALSE(_pos3x.closest_normal_point(Point(-9, 2, 4), result));
  EXPECT_EQ(result, Point(-9, 0, 0));

  EXPECT_TRUE(_negy.closest_normal_point(Point(0, -5), result));
  EXPECT_EQ(result, Point(0, -5));
  EXPECT_TRUE(_negy.closest_normal_point(Point(-10, 0), result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_negy.closest_normal_point(Point(-1, -5), result));
  EXPECT_EQ(result, Point(0, -5));
  EXPECT_TRUE(_negy.closest_normal_point(Point(-2, -2), result));
  EXPECT_EQ(result, Point(0, -2));
  EXPECT_FALSE(_negy.closest_normal_point(Point(-1, 6), result));
  EXPECT_EQ(result, Point(0, 6));
  EXPECT_FALSE(_negy.closest_normal_point(Point(2, -9), result));
  EXPECT_EQ(result, Point(0, -9));
  EXPECT_TRUE(_neg3y.closest_normal_point(Point(0, -5, 0), result));
  EXPECT_EQ(result, Point(0, -5, 0));
  EXPECT_FALSE(_neg3y.closest_normal_point(Point(0, 5, 0), result));
  EXPECT_EQ(result, Point(0, 5, 0));
  EXPECT_FALSE(_neg3y.closest_normal_point(Point(0, -10, 0), result));
  EXPECT_EQ(result, Point(0, -10, 0));
  EXPECT_FALSE(_neg3y.closest_normal_point(Point(2, -9, -4), result));
  EXPECT_EQ(result, Point(0, -9, 0));

  EXPECT_TRUE(_posdiag.closest_normal_point(Point(0, 0), result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_posdiag.closest_normal_point(Point(1, 0), result));
  EXPECT_EQ(result, Point(0.5, 0.5));
  EXPECT_TRUE(_posdiag.closest_normal_point(Point(0, 2), result));
  EXPECT_EQ(result, Point(1, 1));
  EXPECT_FALSE(_posdiag.closest_normal_point(Point(0, 12), result));
  EXPECT_EQ(result, Point(6, 6));
  EXPECT_TRUE(_pos3diag.closest_normal_point(Point(0, 0, 0), result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3diag.closest_normal_point(Point(0, 0, 4), result));
  EXPECT_EQ(result, Point(4. / 3., 4. / 3., 4. / 3.));
  EXPECT_TRUE(_pos3diag.closest_normal_point(Point(0, 4, 4), result));
  EXPECT_EQ(result, Point(8. / 3., 8. / 3., 8. / 3.));

  // negative diagonal end cases
  EXPECT_EQ(_negdiag.closest_point(Point(0, 1)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(1, 1)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(0, 0)), Point(0, 0));
  EXPECT_EQ(_negdiag.closest_point(Point(-6, -6)), Point(-5, -5));
  EXPECT_EQ(_negdiag.closest_point(Point(-7, -6)), Point(-5, -5));
  EXPECT_EQ(_negdiag.closest_point(Point(-6, -9)), Point(-5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 1, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(1, 1, 1)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 0, 0)), Point(0, 0, 0));
  EXPECT_EQ(_neg3diag.closest_point(Point(-6, -6, -6)), Point(-5, -5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(-7, -6, -9)), Point(-5, -5, -5));
  EXPECT_EQ(_neg3diag.closest_point(Point(-6, -9, -5)), Point(-5, -5, -5));
  // middle of the line
  EXPECT_EQ(_negdiag.closest_point(Point(-2, -2)), Point(-2, -2));
  EXPECT_EQ(_negdiag.closest_point(Point(0, -2)), Point(-1, -1));
  EXPECT_EQ(_negdiag.closest_point(Point(-2, 0)), Point(-1, -1));
  EXPECT_EQ(_negdiag.closest_point(Point(-1, -3)), Point(-2, -2));
  EXPECT_EQ(_neg3diag.closest_point(Point(-2, -2, -2)), Point(-2, -2, -2));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, 0, -4)), Point(-4. / 3., -4. / 3., -4. / 3.));
  EXPECT_EQ(_neg3diag.closest_point(Point(0, -4, -4)), Point(-8. / 3., -8. / 3., -8. / 3.));

  EXPECT_TRUE(_negdiag.closest_normal_point(Point(0, 0), result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_negdiag.closest_normal_point(Point(-1, 0), result));
  EXPECT_EQ(result, Point(-0.5, -0.5));
  EXPECT_TRUE(_negdiag.closest_normal_point(Point(0, -2), result));
  EXPECT_EQ(result, Point(-1, -1));
  EXPECT_FALSE(_negdiag.closest_normal_point(Point(0, -12), result));
  EXPECT_EQ(result, Point(-6, -6));
}

TEST_F(LineSegmentTest, containsPointTest)
{
  EXPECT_TRUE(_posx.contains_point(Point(0, 0)));
  EXPECT_TRUE(_posx.contains_point(Point(4, 0)));
  EXPECT_FALSE(_posx.contains_point(Point(3, 3)));
  EXPECT_FALSE(_posx.contains_point(Point(-3, 3)));
  EXPECT_FALSE(_posx.contains_point(Point(-3, -3)));
  EXPECT_FALSE(_posx.contains_point(Point(-.1, 0)));
  EXPECT_TRUE(_pos3x.contains_point(Point(0, 0, 0)));
  EXPECT_TRUE(_pos3x.contains_point(Point(4, 0, 0)));
  EXPECT_FALSE(_pos3x.contains_point(Point(3, 3, 3)));
  EXPECT_FALSE(_pos3x.contains_point(Point(-3, 3, 3)));
  EXPECT_FALSE(_pos3x.contains_point(Point(-3, -3, 3)));
  EXPECT_FALSE(_pos3x.contains_point(Point(-3, -3, -3)));
  EXPECT_FALSE(_pos3x.contains_point(Point(-.1, 0, .1)));
  EXPECT_TRUE(_pos3x.contains_point(Point(.1, 0, 0)));
  EXPECT_FALSE(_pos3x.contains_point(Point(0, 0, .1)));
  EXPECT_FALSE(_pos3x.contains_point(Point(0, .1, .1)));

  EXPECT_TRUE(_posy.contains_point(Point(0, 0)));
  EXPECT_TRUE(_posy.contains_point(Point(0, 4)));
  EXPECT_FALSE(_posy.contains_point(Point(0, 9)));
  EXPECT_FALSE(_posy.contains_point(Point(0, -9)));
  EXPECT_FALSE(_posy.contains_point(Point(3, 3)));
  EXPECT_FALSE(_posy.contains_point(Point(-3, 3)));
  EXPECT_FALSE(_posy.contains_point(Point(3, -3)));
  EXPECT_FALSE(_posy.contains_point(Point(-.1, 0)));

  EXPECT_TRUE(_negy.contains_point(Point(0, 0)));
  EXPECT_TRUE(_negy.contains_point(Point(0, -5)));
  EXPECT_FALSE(_negy.contains_point(Point(4, 0)));
  EXPECT_FALSE(_negy.contains_point(Point(3, 3)));
  EXPECT_FALSE(_negy.contains_point(Point(.1, 0)));
  EXPECT_TRUE(_neg3y.contains_point(Point(0, 0, 0)));
  EXPECT_TRUE(_neg3y.contains_point(Point(0, -5, 0)));
  EXPECT_FALSE(_neg3y.contains_point(Point(4, 0, 0)));
  EXPECT_FALSE(_neg3y.contains_point(Point(0, 4, 0)));
  EXPECT_FALSE(_neg3y.contains_point(Point(0, -9, 0)));
  EXPECT_FALSE(_neg3y.contains_point(Point(3, 3, 3)));
  EXPECT_FALSE(_neg3y.contains_point(Point(-3, 3, 3)));
  EXPECT_FALSE(_neg3y.contains_point(Point(3, -3, 3)));
  EXPECT_FALSE(_neg3y.contains_point(Point(3, -3, -3)));
  EXPECT_FALSE(_neg3y.contains_point(Point(-3, -3, -3)));
  EXPECT_FALSE(_neg3y.contains_point(Point(-.1, 0, .1)));
  EXPECT_FALSE(_neg3y.contains_point(Point(.1, 0, 0)));
  EXPECT_FALSE(_neg3y.contains_point(Point(0, 0, .1)));
  EXPECT_FALSE(_neg3y.contains_point(Point(0, .1, .1)));

  EXPECT_TRUE(_negdiag.contains_point(Point(0, 0)));
  EXPECT_TRUE(_posdiag.contains_point(Point(4, 4)));
  EXPECT_TRUE(_posdiag.contains_point(Point(5, 5)));
  EXPECT_FALSE(_posdiag.contains_point(Point(0, 3)));
  EXPECT_FALSE(_posdiag.contains_point(Point(-.1, 0)));
  EXPECT_TRUE(_pos3diag.contains_point(Point(0, 0, 0)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(0, -5, 0)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(4, 0, 0)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(0, 4, 0)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(0, -9, 0)));
  EXPECT_TRUE(_pos3diag.contains_point(Point(3, 3, 3)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(-3, -3, -3)));
  EXPECT_TRUE(_pos3diag.contains_point(Point(5, 5, 5)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(-5, -5, -5)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(-3, 3, 3)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(3, -3, 3)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(3, -3, -3)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(-.1, 0, .1)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(.1, 0, 0)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(0, 0, .1)));
  EXPECT_FALSE(_pos3diag.contains_point(Point(0, .1, .1)));

  EXPECT_TRUE(_negdiag.contains_point(Point(0, 0)));
  EXPECT_TRUE(_negdiag.contains_point(Point(-5, -5)));
  EXPECT_FALSE(_negdiag.contains_point(Point(0, 3)));
  EXPECT_FALSE(_negdiag.contains_point(Point(-.1, 0)));
  EXPECT_TRUE(_neg3diag.contains_point(Point(0, 0, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(0, -5, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(4, 0, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(0, 4, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(0, -9, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(3, 3, 3)));
  EXPECT_TRUE(_neg3diag.contains_point(Point(-3, -3, -3)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(5, 5, 5)));
  EXPECT_TRUE(_neg3diag.contains_point(Point(-5, -5, -5)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(-3, 3, 3)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(3, -3, 3)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(3, -3, -3)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(-.1, 0, .1)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(.1, 0, 0)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(0, 0, .1)));
  EXPECT_FALSE(_neg3diag.contains_point(Point(0, .1, .1)));
}

TEST_F(LineSegmentTest, planeIntersectTest)
{
  Point result;
  Plane xy;
  xy.xy_plane(1);
  Plane xz;
  xz.xz_plane(1);
  Plane yz;
  yz.yz_plane(1);
  Plane diag(Point(0, 0, 0), Point(1, 1, 1), Point(-1, 1, 0));

  // Test all the 3D LineSegments against all 4 planes
  EXPECT_FALSE(_pos3x.intersect(xy, result));
  EXPECT_FALSE(_pos3x.intersect(xz, result));
  EXPECT_TRUE(_pos3x.intersect(yz, result));
  EXPECT_EQ(result, Point(1, 0, 0));
  EXPECT_TRUE(_pos3x.intersect(diag, result));
  EXPECT_EQ(result, Point(0, 0, 0));

  EXPECT_FALSE(_neg3y.intersect(xy, result));
  EXPECT_FALSE(_neg3y.intersect(xz, result));
  EXPECT_FALSE(_neg3y.intersect(yz, result));
  EXPECT_TRUE(_neg3y.intersect(diag, result));
  EXPECT_EQ(result, Point(0, 0, 0));

  EXPECT_TRUE(_pos3diag.intersect(xy, result));
  EXPECT_EQ(result, Point(1, 1, 1));
  EXPECT_TRUE(_pos3diag.intersect(xz, result));
  EXPECT_EQ(result, Point(1, 1, 1));
  EXPECT_TRUE(_pos3diag.intersect(yz, result));
  EXPECT_EQ(result, Point(1, 1, 1));
  EXPECT_TRUE(_pos3diag.intersect(diag, result));

  EXPECT_FALSE(_neg3diag.intersect(xy, result));
  EXPECT_FALSE(_neg3diag.intersect(xz, result));
  EXPECT_FALSE(_neg3diag.intersect(yz, result));
  EXPECT_TRUE(_neg3diag.intersect(diag, result));

  // Make some special lines to test
  LineSegment t1(Point(1, 1, 1), Point(-1, 1, 0));
  EXPECT_TRUE(t1.intersect(diag, result));

  LineSegment t2(Point(1, 1, 1), Point(1, 1, 0));
  EXPECT_TRUE(t2.intersect(diag, result));
  EXPECT_EQ(result, Point(1, 1, 1));

  LineSegment t3(Point(1, 1, 1), Point(1, 1, 0));
  EXPECT_TRUE(t3.intersect(diag, result));
  EXPECT_EQ(result, Point(1, 1, 1));

  LineSegment t4(Point(1, 1, 1), Point(-4, 5, 9));
  EXPECT_TRUE(t4.intersect(diag, result));
  EXPECT_EQ(result, Point(1, 1, 1));

  LineSegment t5(Point(-1, 1, 0), Point(4, -5, -6));
  EXPECT_TRUE(t5.intersect(diag, result));
  EXPECT_EQ(result, Point(-1, 1, 0));

  LineSegment t6(Point(0, 0, 0), Point(-94, -5, -6));
  EXPECT_TRUE(t6.intersect(diag, result));
  EXPECT_EQ(result, Point(0, 0, 0));

  // Test lines parallel to the plane (first two inside the plane, second two below it)
  LineSegment t7(Point(3, 4, 1), Point(-9, 5, 1));
  EXPECT_TRUE(t7.intersect(xy, result));

  LineSegment t8(Point(-3, 4, 1), Point(9, -5, 1));
  EXPECT_TRUE(t8.intersect(xy, result));

  LineSegment t9(Point(3, 4, 0), Point(-9, 5, 0));
  EXPECT_FALSE(t9.intersect(xy, result));

  LineSegment t10(Point(-3, 4, 0), Point(9, -5, 0));
  EXPECT_FALSE(t10.intersect(xy, result));
}

TEST_F(LineSegmentTest, lineIntersectTest)
{
  Point result;
  EXPECT_TRUE(_posx.intersect(_negy, result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_posdiag.intersect(_negy, result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_negdiag.intersect(_negy, result));
  EXPECT_EQ(result, Point(0, 0));
  EXPECT_TRUE(_posdiag.intersect(_negdiag, result));

  EXPECT_TRUE(_pos3x.intersect(_neg3y, result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3diag.intersect(_neg3y, result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_neg3diag.intersect(_neg3y, result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3diag.intersect(_neg3diag, result));

  LineSegment t1(Point(0, 1, 0), Point(1, 1, 0));
  EXPECT_FALSE(t1.intersect(_pos3x, result));
  EXPECT_FALSE(t1.intersect(_pos3diag, result));
  EXPECT_FALSE(t1.intersect(_neg3diag, result));
  // The lines formed by the line segments do intersect but not the line segments themselves
  EXPECT_FALSE(t1.intersect(_neg3y, result));

  LineSegment t2(Point(0, 0, 0), Point(0, 0, 1));
  /*bool ans = */ t1.intersect(t2, result);
  EXPECT_FALSE(t1.intersect(t2, result));
  EXPECT_TRUE(t2.intersect(_pos3diag, result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(t2.intersect(_neg3diag, result));
  EXPECT_EQ(result, Point(0, 0, 0));

  LineSegment t3(Point(1, 1, 1), Point(4, -5, 7));
  EXPECT_TRUE(t3.intersect(_pos3diag, result));
  EXPECT_EQ(result, Point(1, 1, 1));
  EXPECT_FALSE(t3.intersect(_neg3diag, result));
  EXPECT_FALSE(t3.intersect(t2, result));

  LineSegment t4(Point(-3, -3, -3), Point(4, -5, 7));
  EXPECT_FALSE(t4.intersect(_pos3diag, result));
  EXPECT_TRUE(t4.intersect(_neg3diag, result));
  EXPECT_EQ(result, Point(-3, -3, -3));
  EXPECT_TRUE(t4.intersect(t3, result));
  EXPECT_EQ(result, Point(4, -5, 7));

  LineSegment t5(Point(1, 0, 0), Point(-4, 0, 0));
  EXPECT_TRUE(t5.intersect(_pos3x, result));
  EXPECT_FALSE(t5.intersect(t4, result));
  EXPECT_TRUE(t5.intersect(_neg3y, result));
  EXPECT_NEAR(result(0), 0., 1e-16);
  EXPECT_NEAR(result(1), 0., 1e-16);
  EXPECT_NEAR(result(2), 0., 1e-16);

  // Switch some of the a.intersect(b) tests to b.intersect(a) to make sure we get the same result
  EXPECT_FALSE(t2.intersect(t1, result));
  EXPECT_FALSE(_pos3diag.intersect(t4, result));
  EXPECT_TRUE(_neg3diag.intersect(t4, result));
  EXPECT_EQ(result, Point(-3, -3, -3));
  EXPECT_TRUE(t3.intersect(t4, result));
  EXPECT_EQ(result, Point(4, -5, 7));
  EXPECT_TRUE(_neg3y.intersect(_pos3x, result));
  EXPECT_EQ(result, Point(0, 0, 0));
  EXPECT_TRUE(_pos3x.intersect(t5, result));
  EXPECT_FALSE(t4.intersect(t5, result));
  EXPECT_TRUE(_neg3y.intersect(t5, result));
  EXPECT_EQ(result, Point(0, 0, 0));
}

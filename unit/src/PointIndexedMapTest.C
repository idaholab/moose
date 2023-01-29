//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointIndexedMap.h"
#include "gtest/gtest.h"

TEST(PointIndexedMap, bracket)
{
  PointIndexedMap map(Point(1, 1, 3));
  Point p0(0, 0, 1);
  map[p0] = 12;
  EXPECT_EQ(map[p0], 12);
}

TEST(PointIndexedMap, hasKey)
{
  PointIndexedMap map(Point(1, 1, 3));
  Point p0(0, 0, 1);
  map[p0] = 12;
  EXPECT_TRUE(map.hasKey(p0));
}

TEST(PointIndexedMap, find)
{
  auto ref = 12.;
  std::vector<Point> points;
  points.push_back(Point(0, 0, 0));
  points.push_back(Point(1e-11, 1e-11, 1e-11));
  points.push_back(Point(5e-11, 5e-11, 5e-11));
  points.push_back(Point(1e-5, 1e-5, 1e-5));
  points.push_back(Point(1, 1, 1));
  points.push_back(Point(10, 11, 12));
  points.push_back(Point(10.1342343241231, 11.123453432132134455689, 12.1234356787654322));
  points.push_back(Point(1.2345678901235, 1.2345678901235, 1.2345678901235));
  points.push_back(Point(-1.2345678901235, -1.2345678901235, -1.2345678901235));
  // Offset from bin edge
  points.push_back(Point(1.2345678901235233, 1.2345678901235341, 1.2345678901235653));
  points.push_back(Point(-1.2345678901235233, -1.2345678901235341, -1.2345678901235653));

  const Real mesh_size_z = 21;

  for (auto p0 : points)
  {
    PointIndexedMap map(Point(20, 20, mesh_size_z));

    std::vector<Real> epsilons{1e-14, 1e-15};
    for (auto eps_base : epsilons)
    {
      auto eps = (p0(0) != 0) ? eps_base * p0(0) : eps_base;
      map[p0] = ref;
      EXPECT_TRUE(map.find(p0) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, 0, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, -eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, 0, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, 0, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, eps, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, -eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(eps, -eps, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, 0, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, -eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, 0, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, 0, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, eps, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, -eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(-eps, -eps, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, 0, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, -eps, 0)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, 0, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, 0, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, eps, -eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, -eps, eps)) != map.end());
      EXPECT_TRUE(map.find(p0 + Point(0, -eps, -eps)) != map.end());
    }
    // We should be beyond the tolerance of the map search
    if (p0(2) != 0 && p0(2) * 1e-9 > 1e-12 * mesh_size_z)
    {
      EXPECT_TRUE(map.find(p0 + Point(0, 0, 1e-9 * p0(2))) == map.end());
    }
  }
}

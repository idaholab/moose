//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"
#include "KDTree.h"

#define TOL 1e-10

/**
 * Test KDTree proxy object that uses underlying nanoflann implementation
 */
TEST(KDTree, radiusSearch)
{
  std::vector<Point> master_points = {Point(0.0, -0.2, 0.1),
                                      Point(0.0, 0.0, 0.1),
                                      Point(0.2, 0.3, 0.1),
                                      Point(0.2, 0.3, 0.2),
                                      Point(1.0, 0.3, 0.2)};
  KDTree _kd_tree(master_points, 50);
  std::vector<std::pair<std::size_t, Real>> indices_dist;

  // get all points
  Point origin(-0.1, -0.2, 0.1);
  _kd_tree.radiusSearch(origin, 10.0, indices_dist);
  EXPECT_EQ(5, indices_dist.size());
  for (unsigned int j = 0; j < 5; ++j)
  {
    EXPECT_EQ(j, indices_dist[j].first);
    EXPECT_NEAR((master_points[j] - origin).norm_sq(), indices_dist[j].second, TOL);
  }

  // all except the last one
  _kd_tree.radiusSearch(origin, 1.2, indices_dist);
  EXPECT_EQ(4, indices_dist.size());
  for (unsigned int j = 0; j < 4; ++j)
  {
    EXPECT_EQ(j, indices_dist[j].first);
    EXPECT_NEAR((master_points[j] - origin).norm_sq(), indices_dist[j].second, TOL);
  }

  // just one
  _kd_tree.radiusSearch(origin, 0.1001, indices_dist);
  EXPECT_EQ(1, indices_dist.size());
  EXPECT_EQ(0, indices_dist[0].first);
  EXPECT_NEAR((master_points[0] - origin).norm_sq(), indices_dist[0].second, TOL);
}

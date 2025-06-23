//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "BSpline.h"

TEST(BSplineTest, buildKnotVectorTest)
{
  const unsigned int degree = 1;
  const std::vector<libMesh::Point> control_points = {{0, 0, 0}, {1, 0, 0}};

  Moose::BSpline b_spline(degree, control_points);
  const std::vector<double> expected_knot_vector = {0, 0, 1, 1};
  ASSERT_EQ(b_spline.getKnotVector(), expected_knot_vector);
}

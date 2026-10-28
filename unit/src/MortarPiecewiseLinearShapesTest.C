//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MortarUtils.h"

#include "libmesh/elem.h"

#include <vector>

namespace
{
// Roundoff allowance for values of order one
constexpr Real tol = 1e-12;

std::vector<Point>
samplePoints(const ElemType type)
{
  if (type == EDGE3)
    return {Point(-0.9), Point(-0.3), Point(0.2), Point(0.7)};
  if (type == TRI6)
    return {Point(0.1, 0.1), Point(0.3, 0.3), Point(0.6, 0.1), Point(0.1, 0.7), Point(0.2, 0.45)};
  return {Point(-0.8, -0.7),
          Point(0.1, -0.2),
          Point(0.6, 0.3),
          Point(-0.4, 0.8),
          Point(0.9, -0.85),
          Point(0.3, 0.6)};
}
}

TEST(MortarPiecewiseLinearShapesTest, InterpolatesNodesAndPartitionsUnity)
{
  for (const auto type : {EDGE3, TRI6, QUAD8, QUAD9})
  {
    const auto elem = Elem::build(type);
    std::vector<Real> phi;

    for (const auto node : make_range(elem->n_nodes()))
    {
      Moose::Mortar::piecewiseLinearShapes(*elem, elem->master_point(node), phi);
      for (const auto i : index_range(phi))
        EXPECT_NEAR(phi[i], i == node ? 1 : 0, tol) << "type " << type << " node " << node;
    }

    for (const auto & point : samplePoints(type))
    {
      Moose::Mortar::piecewiseLinearShapes(*elem, point, phi);
      Real sum = 0;
      for (const auto i : index_range(phi))
      {
        EXPECT_GE(phi[i], -tol);
        sum += phi[i];
      }
      EXPECT_NEAR(sum, 1, tol);
    }
  }
}

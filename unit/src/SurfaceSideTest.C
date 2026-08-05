//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SurfaceSide.h"

#include "libmesh/int_range.h"

#include <algorithm>
#include <array>

// signedValueSideness with the default signed-distance convention (negative inside).
TEST(SurfaceSideTest, SignedValueInsideIsNegative)
{
  const Real tol = 0.1;

  // Clearly interior / exterior values.
  EXPECT_EQ(signedValueSideness(-1.0, tol, true), SurfaceSide::INSIDE);
  EXPECT_EQ(signedValueSideness(1.0, tol, true), SurfaceSide::OUTSIDE);

  // Inside the on-surface band, including both band endpoints.
  EXPECT_EQ(signedValueSideness(0.0, tol, true), SurfaceSide::ON);
  EXPECT_EQ(signedValueSideness(-tol, tol, true), SurfaceSide::ON);
  EXPECT_EQ(signedValueSideness(tol, tol, true), SurfaceSide::ON);

  // Just outside the band on either side.
  EXPECT_EQ(signedValueSideness(-1.001 * tol, tol, true), SurfaceSide::INSIDE);
  EXPECT_EQ(signedValueSideness(1.001 * tol, tol, true), SurfaceSide::OUTSIDE);
}

// signedValueSideness with inside_is_negative == false flips interior/exterior.
TEST(SurfaceSideTest, SignedValueInsideIsPositive)
{
  const Real tol = 0.1;

  EXPECT_EQ(signedValueSideness(1.0, tol, false), SurfaceSide::INSIDE);
  EXPECT_EQ(signedValueSideness(-1.0, tol, false), SurfaceSide::OUTSIDE);
  EXPECT_EQ(signedValueSideness(0.0, tol, false), SurfaceSide::ON);
}

// unionSideness applies the OUTSIDE < ON < INSIDE precedence: the union side is the
// higher-precedence of the two arguments, symmetrically. This is the truth table
// PointInUnionCheckUO folds over per provider.
TEST(SurfaceSideTest, Union)
{
  // Listed in increasing precedence, so the index doubles as the precedence rank and
  // the expected union of any pair is the side at the larger of the two ranks.
  const std::array by_precedence = {SurfaceSide::OUTSIDE, SurfaceSide::ON, SurfaceSide::INSIDE};

  for (const auto i : index_range(by_precedence))
    for (const auto j : index_range(by_precedence))
      EXPECT_EQ(unionSideness(by_precedence[i], by_precedence[j]), by_precedence[std::max(i, j)]);
}

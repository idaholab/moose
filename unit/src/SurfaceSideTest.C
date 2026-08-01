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

// unionSideness follows the INSIDE > ON > OUTSIDE precedence.
TEST(SurfaceSideTest, Union)
{
  // Empty union classifies every point as outside.
  EXPECT_EQ(unionSideness({}), SurfaceSide::OUTSIDE);

  // All outside stays outside.
  EXPECT_EQ(unionSideness({SurfaceSide::OUTSIDE, SurfaceSide::OUTSIDE}), SurfaceSide::OUTSIDE);

  // Any ON (and no inside) is on.
  EXPECT_EQ(unionSideness({SurfaceSide::OUTSIDE, SurfaceSide::ON}), SurfaceSide::ON);
  EXPECT_EQ(unionSideness({SurfaceSide::ON, SurfaceSide::ON}), SurfaceSide::ON);

  // Any INSIDE wins over ON and OUTSIDE, regardless of order.
  EXPECT_EQ(unionSideness({SurfaceSide::ON, SurfaceSide::INSIDE}), SurfaceSide::INSIDE);
  EXPECT_EQ(unionSideness({SurfaceSide::INSIDE, SurfaceSide::OUTSIDE}), SurfaceSide::INSIDE);
}

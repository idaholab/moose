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
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(-1.0, tol, true),
            SurfaceGeometry::SurfaceSide::INSIDE);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(1.0, tol, true),
            SurfaceGeometry::SurfaceSide::OUTSIDE);

  // Inside the on-surface band, including both band endpoints.
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(0.0, tol, true), SurfaceGeometry::SurfaceSide::ON);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(-tol, tol, true),
            SurfaceGeometry::SurfaceSide::ON);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(tol, tol, true), SurfaceGeometry::SurfaceSide::ON);

  // Just outside the band on either side.
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(-1.001 * tol, tol, true),
            SurfaceGeometry::SurfaceSide::INSIDE);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(1.001 * tol, tol, true),
            SurfaceGeometry::SurfaceSide::OUTSIDE);
}

// signedValueSideness with inside_is_negative == false flips interior/exterior.
TEST(SurfaceSideTest, SignedValueInsideIsPositive)
{
  const Real tol = 0.1;

  EXPECT_EQ(SurfaceGeometry::signedValueSideness(1.0, tol, false),
            SurfaceGeometry::SurfaceSide::INSIDE);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(-1.0, tol, false),
            SurfaceGeometry::SurfaceSide::OUTSIDE);
  EXPECT_EQ(SurfaceGeometry::signedValueSideness(0.0, tol, false),
            SurfaceGeometry::SurfaceSide::ON);
}

// unionSideness applies the OUTSIDE < ON < INSIDE precedence: the union side is the
// higher-precedence of the two arguments, symmetrically. This is the truth table
// PointInUnionCheckUO folds over per provider.
TEST(SurfaceSideTest, Union)
{
  // Listed in increasing precedence, so the index doubles as the precedence rank and
  // the expected union of any pair is the side at the larger of the two ranks.
  const std::array by_precedence = {SurfaceGeometry::SurfaceSide::OUTSIDE,
                                    SurfaceGeometry::SurfaceSide::ON,
                                    SurfaceGeometry::SurfaceSide::INSIDE};

  for (const auto i : index_range(by_precedence))
    for (const auto j : index_range(by_precedence))
      EXPECT_EQ(SurfaceGeometry::unionSideness(by_precedence[i], by_precedence[j]),
                by_precedence[std::max(i, j)]);
}

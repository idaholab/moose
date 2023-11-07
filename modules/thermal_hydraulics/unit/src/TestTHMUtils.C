//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "libmesh/vector_value.h"
#include "MooseUtils.h"
#include "THMUtils.h"
#include "THMTestUtils.h"

TEST(NumericsTest, computeOrthogonalDirections_x_aligned)
{
  const RealVectorValue n(-2, 0, 0);
  RealVectorValue t1, t2;
  THM::computeOrthogonalDirections(n, t1, t2);

  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1.norm(), 1));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t2.norm(), 1));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1 * n, 0));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t2 * n, 0));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1 * t2, 0));
}

TEST(NumericsTest, computeOrthogonalDirections_skew)
{
  const RealVectorValue n(1, 2, 3);
  RealVectorValue t1, t2;
  THM::computeOrthogonalDirections(n, t1, t2);

  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1.norm(), 1));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t2.norm(), 1));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1 * n, 0));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t2 * n, 0));
  EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(t1 * t2, 0));
}

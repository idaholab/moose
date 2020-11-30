//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowCubic.h"

const double eps = 1.0E-8;

/// Test cubic
TEST(PorousFlowCubic, cubic)
{
  EXPECT_NEAR(3.0, PorousFlowCubic::cubic(2.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(4.0,
              (PorousFlowCubic::cubic(2.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(2.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
  EXPECT_NEAR(6.0, PorousFlowCubic::cubic(5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(7.0,
              (PorousFlowCubic::cubic(5.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
  EXPECT_NEAR(-6.0, PorousFlowCubic::cubic(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(4.0, PorousFlowCubic::cubic(3.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(3.0, PorousFlowCubic::cubic(4.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(48.0, PorousFlowCubic::cubic(7.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
}

/// Test dcubic
TEST(PorousFlowCubic, dcubic)
{
  EXPECT_NEAR(4.0, PorousFlowCubic::dcubic(2.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(7.0, PorousFlowCubic::dcubic(5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0), 1.0E-12);
  EXPECT_NEAR(PorousFlowCubic::dcubic(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0),
              (PorousFlowCubic::cubic(1.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
  EXPECT_NEAR(PorousFlowCubic::dcubic(3.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0),
              (PorousFlowCubic::cubic(3.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(3.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
  EXPECT_NEAR(PorousFlowCubic::dcubic(4.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0),
              (PorousFlowCubic::cubic(4.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(4.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
  EXPECT_NEAR(PorousFlowCubic::dcubic(7.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0),
              (PorousFlowCubic::cubic(7.0 + eps, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0) -
               PorousFlowCubic::cubic(7.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0)) /
                  eps,
              1.0E-5);
}

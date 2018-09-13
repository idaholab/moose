//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "DimensionlessFlowNumbers.h"

TEST(DimensionlessFlowNumbersTest, reynolds)
{
  EXPECT_EQ(fp::reynolds(1000., -1., 1., 1.), 1000.);
  EXPECT_NEAR(fp::reynolds(998., 2.5, 1e-4, 8.9e-4), 280.3370786516856, 1e-12);
}

TEST(DimensionlessFlowNumbersTest, prandtl)
{
  EXPECT_EQ(fp::prandtl(10., 10., 100.), 1.);
  EXPECT_NEAR(fp::prandtl(4184., 8.9e-4, 1.0020), 3.71632734530938, 1e-12);
}

TEST(DimensionlessFlowNumbersTest, grashof)
{
  EXPECT_EQ(fp::grashof(1., 1., 2., 1., 1., 1., 1.), 1.);
  EXPECT_NEAR(fp::grashof(0.000214, 2.5, 0, 2.234, 998., 8.9e-4, 9.81), 73578912563.777863, 1e-12);
}

TEST(DimensionlessFlowNumbersTest, laplace)
{
  EXPECT_EQ(fp::laplace(1., 1., 1., 1.), 1.);
  EXPECT_NEAR(fp::laplace(72.8e-3, 998., 2.234, 8.9e-4), 204910907.20868585, 1e-12);
}

TEST(DimensionlessFlowNumbersTest, thermal_diffusivity)
{
  EXPECT_EQ(fp::thermalDiffusivity(1., 1., 1.), 1.);
  EXPECT_NEAR(fp::thermalDiffusivity(1.5, 0.15, 2416.0), 0.0041390728476821195, 1e-12);
}

TEST(DimensionlessFlowNumbersTest, peclet)
{
  EXPECT_EQ(fp::peclet(1., 1., 1.), 1.);
  EXPECT_NEAR(fp::peclet(10., 0.014, 1.5), 0.09333333333333334, 1e-12);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "HeatTransferUtils.h"

TEST(HeatTransferUtils, reynolds)
{
  const Real rho = 998.0;
  const Real vel = -2.5;
  const Real L = 1e-4;
  const Real mu = 8.9e-4;
  EXPECT_NEAR(HeatTransferUtils::reynolds(rho, vel, L, mu), 280.3370786516856, 1e-12);
}

TEST(HeatTransferUtils, prandtl)
{
  const Real cp = 4184.0;
  const Real mu = 8.9e-4;
  const Real k = 1.0020;
  EXPECT_NEAR(HeatTransferUtils::prandtl(cp, mu, k), 3.71632734530938, 1e-12);
}

TEST(HeatTransferUtils, grashof)
{
  const Real beta = 0.000214;
  const Real T_s = 2.5;
  const Real T_bulk = 0;
  const Real L = 2.234;
  const Real rho = 998.0;
  const Real mu = 8.9e-4;
  const Real g = 9.81;
  EXPECT_NEAR(
      HeatTransferUtils::grashof(beta, T_s, T_bulk, L, rho, mu, g), 73578912563.777863, 1e-12);
}

TEST(HeatTransferUtils, laplace)
{
  const Real sigma = 72.8e-3;
  const Real rho = 998.0;
  const Real L = 2.234;
  const Real mu = 8.9e-4;
  EXPECT_NEAR(HeatTransferUtils::laplace(sigma, rho, L, mu), 204910907.20868585, 1e-12);
}

TEST(HeatTransferUtils, thermal_diffusivity)
{
  const Real k = 1.5;
  const Real rho = 0.15;
  const Real cp = 2416.0;
  EXPECT_NEAR(HeatTransferUtils::thermalDiffusivity(k, rho, cp), 0.0041390728476821195, 1e-12);
}

TEST(HeatTransferUtils, peclet)
{
  const Real vel = -10.0;
  const Real L = 0.014;
  const Real diffusivity = 1.5;
  EXPECT_NEAR(HeatTransferUtils::peclet(vel, L, diffusivity), 0.09333333333333334, 1e-12);
}

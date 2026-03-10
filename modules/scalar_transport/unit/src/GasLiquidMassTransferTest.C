//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GasLiquidMassTransferTest.h"

#include <cmath>

const double tol = 1.0e-6;

const double p = 1e5;
const double T = 800;
const double M = 30e-3;
const double R = 8.31446;
const double mu = 20e-6;
const double v = 1.0;
const double d = 10e-3;
const double radius = 1e-3;

const double rho = p / R / T * M;
const double Re = rho * v * d / mu;

TEST_F(GasLiquidMassTransferTest, test)
{

  // Stokes-Einstein
  {
    const double D = 1.38064852e-23 * T / (6.0 * libMesh::pi * mu * radius);
    const double Sc = mu / (rho * D);
    const double mtc_gold = 0.023 * std::pow(Re, 0.8) * std::pow(Sc, 0.4) * D / d;
    EXPECT_LT(std::abs(_mtc_stokes->mtc(p, T, v) - mtc_gold), tol * mtc_gold);
  }

  // Wilke-Chang
  {
    const double molar_volume = M / (rho / 1e6); // cm3/mol
    const double mu_cp = mu * 1e3 / 1e2 * 1e2;   // cP
    const double D = 7.4e-8 * T * std::pow(1 * M * 1e3, 0.5) /
                     (mu_cp * std::pow(molar_volume, 0.6)) * std::pow(0.01, 2);
    const double Sc = mu / (rho * D);
    const double mtc_gold = 0.023 * std::pow(Re, 0.8) * std::pow(Sc, 0.4) * D / d;
    EXPECT_LT(std::abs(_mtc_wilkes->mtc(p, T, v) - mtc_gold), tol * mtc_gold);
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesPTTest.h"
#include "SinglePhaseFluidPropertiesPTUtils.h"

/**
 * Verify that the fluid name is correctly returned
 */
TEST_F(IdealGasFluidPropertiesPTTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "ideal_gas"); }

/**
 * Verify that the default molar mass is correctly returned
 */
TEST_F(IdealGasFluidPropertiesPTTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 2.9e-2, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the fluid properties
 */
TEST_F(IdealGasFluidPropertiesPTTest, properties)
{
  const Real molar_mass = 0.029;
  const Real cv = 718.0;
  const Real cp = 1005.0;
  const Real thermal_conductivity = 0.02568;
  const Real entropy = 6850.0;
  const Real viscosity = 18.23e-6;
  const Real henry = 0.0;
  const Real R = 8.3144598;

  const Real tol = REL_TOL_CONSISTENCY;

  Real p, T;

  p = 1.0e6;
  T = 300.0;
  REL_TEST(_fp->beta(p, T), 1.0 / T, tol);
  REL_TEST(_fp->cp(p, T), cp, tol);
  REL_TEST(_fp->cv(p, T), cv, tol);
  REL_TEST(_fp->c(p, T), std::sqrt(cp * R * T / (cv * molar_mass)), tol);
  REL_TEST(_fp->k(p, T), thermal_conductivity, tol);
  REL_TEST(_fp->k(p, T), thermal_conductivity, tol);
  REL_TEST(_fp->s(p, T), entropy, tol);
  REL_TEST(_fp->rho(p, T), p * molar_mass / (R * T), tol);
  REL_TEST(_fp->e(p, T), cv * T, tol);
  REL_TEST(_fp->mu(p, T), viscosity, tol);
  REL_TEST(_fp->mu(p, T), viscosity, tol);
  REL_TEST(_fp->h(p, T), cp * T, tol);
  ABS_TEST(_fp->henryConstant(T), henry, tol);
}

/**
 * Verify calculation of the derivatives by comparing with finite
 * differences
 */
TEST_F(IdealGasFluidPropertiesPTTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 1.0e6;
  const Real T = 300.0;

  DERIV_TEST(_fp->rho, _fp->rho_dpT, p, T, tol);
  DERIV_TEST(_fp->mu, _fp->mu_dpT, p, T, tol);
  DERIV_TEST(_fp->e, _fp->e_dpT, p, T, tol);
  DERIV_TEST(_fp->h, _fp->h_dpT, p, T, tol);
  DERIV_TEST(_fp->k, _fp->k_dpT, p, T, tol);

  Real henry, dhenry_dT;
  _fp->henryConstant_dT(T, henry, dhenry_dT);
  ABS_TEST(dhenry_dT, 0.0, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(IdealGasFluidPropertiesPTTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;

  combinedProperties(_fp, p, T, REL_TOL_SAVED_VALUE);
}

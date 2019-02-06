//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaClFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Verify that critical properties are correctly returned
 */
TEST_F(NaClFluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 1.82e7, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 3841.15, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 108.43, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that triple point properties are correctly returned
 */
TEST_F(NaClFluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 50.0, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 1073.85, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(NaClFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "nacl"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(NaClFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 58.443e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the NACL properties the solid halite phase.
 * Density data from Brown, "The NaCl pressure standard", J. Appl. Phys., 86 (1999).
 *
 * Values for cp and enthalpy are difficult to compare against. Instead, the
 * values provided by the BrineFluidProperties UserObject were compared against
 * simple correlations, eg. from NIST sodium chloride data.
 *
 * Values for thermal conductivity from Urqhart and Bauer,
 * Experimental determination of single-crystal halite thermal conductivity,
 * diffusivity and specific heat from -75 C to 300 C, Int. J. Rock Mech.
 * and Mining Sci., 78 (2015)
 */
TEST_F(NaClFluidPropertiesTest, halite)
{
  // Density and cp
  Real p0, p1, p2, T0, T1, T2;

  const Real tol = REL_TOL_EXTERNAL_VALUE;

  p0 = 30.0e6;
  p1 = 60.0e6;
  p2 = 80.0e6;
  T0 = 300.0;
  T1 = 500.0;
  T2 = 700.0;

  REL_TEST(_fp->rho_from_p_T(p0, T0), 2167.88, tol);
  REL_TEST(_fp->rho_from_p_T(p1, T1), 2116.0, tol);
  REL_TEST(_fp->rho_from_p_T(p2, T2), 2056.8, tol);
  REL_TEST(_fp->cp_from_p_T(p0, T0), 0.865e3, 40.0 * tol);
  REL_TEST(_fp->cp_from_p_T(p1, T1), 0.922e3, 40.0 * tol);
  REL_TEST(_fp->cp_from_p_T(p2, T2), 0.979e3, 40.0 * tol);

  // Test enthalpy at the triple point pressure of water
  Real pt = 611.657;

  ABS_TEST(_fp->h_from_p_T(pt, 273.16), 0.0, tol);
  REL_TEST(_fp->h_from_p_T(pt, 573.15), 271.13e3, tol);
  REL_TEST(_fp->h_from_p_T(pt, 673.15), 366.55e3, tol);

  // Thermal conductivity (function of T only)
  REL_TEST(_fp->k_from_p_T(p0, 323.15), 5.488, 10.0 * tol);
  REL_TEST(_fp->k_from_p_T(p0, 423.15), 3.911, 10.0 * tol);
  REL_TEST(_fp->k_from_p_T(p0, 523.15), 3.024, 20.0 * tol);
}

/**
 * Verify calculation of the derivatives of halite properties by comparing with finite
 * differences
 */
TEST_F(NaClFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30.0e6;
  const Real T = 300.0;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(NaClFluidPropertiesTest, combined)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const Real p = 1.0e6;
  const Real T = 300.0;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real e, de_dp, de_dT;
  _fp->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT;
  _fp->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}

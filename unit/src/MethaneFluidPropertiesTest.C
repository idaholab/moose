//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MethaneFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Verify that critical properties are correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 4.5992e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 190.564, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 162.66, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that triple point properties are correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 1.169e4, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 90.6941, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "methane"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 16.0425e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that the coefficients for Henry's constant are correct using
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(MethaneFluidPropertiesTest, henry)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> hc = _fp->henryCoefficients();

  REL_TEST(hc[0], -10.44708, tol);
  REL_TEST(hc[1], 4.66491, tol);
  REL_TEST(hc[2], 12.1298, tol);
}

/**
 * Verify calculation of vapor pressure, vapor density and saturated liquid
 * density
 */
TEST_F(MethaneFluidPropertiesTest, vapor)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  // Vapor pressure
  REL_TEST(_fp->vaporPressure(110.0), 0.08813e6, tol);
  REL_TEST(_fp->vaporPressure(130.0), 0.36732e6, tol);
  REL_TEST(_fp->vaporPressure(190.0), 4.5186e6, tol);

  // Saturated vapor density
  REL_TEST(_fp->saturatedVaporDensity(110.0), 1.5982, tol);
  REL_TEST(_fp->saturatedVaporDensity(130.0), 5.9804, tol);
  REL_TEST(_fp->saturatedVaporDensity(190.0), 125.18, tol);

  // Saturated liquid density
  REL_TEST(_fp->saturatedLiquidDensity(110.0), 424.78, tol);
  REL_TEST(_fp->saturatedLiquidDensity(130.0), 394.04, tol);
  REL_TEST(_fp->saturatedLiquidDensity(190.0), 200.78, tol);
}

/**
 * Verify calculation of thermophysical properties of methane using
 * verification data provided in
 * Setzmann and Wagner, A new equation of state and tables of thermodynamic
 * properties for methane covering the range from the melting line to 625 K at
 * pressures up to 100 MPa, Journal of Physical and Chemical Reference Data,
 * 20, 1061--1155 (1991)
 * and
 * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
 * Computer Equations
 */
TEST_F(MethaneFluidPropertiesTest, properties)
{
  // Pressure = 10 MPa, temperature = 350 K
  Real p = 10.0e6;
  Real T = 350.0;
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->rho_from_p_T(p, T), 59.261, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 47.785e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), -120.96e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), -2.1735e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 1.9048e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 2.8021e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 487.29, tol);
  REL_TEST(_fp->mu_from_p_T(p, T), 0.01276e-3, tol);
  REL_TEST(_fp->k_from_p_T(p, T), 0.04113, tol);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(MethaneFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 10.0e6;
  Real T = 350.0;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(MethaneFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;
  const Real tol = REL_TOL_CONSISTENCY;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  _fp->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  _fp->rho_mu_from_p_T(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  _fp->rho_mu_from_p_T(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  _fp->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}

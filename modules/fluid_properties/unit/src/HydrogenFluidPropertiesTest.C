//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HydrogenFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(HydrogenFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "hydrogen"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(HydrogenFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 2.01588e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the critical properties are correctly returned
 */
TEST_F(HydrogenFluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 1.315e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 33.19, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 31.26226704, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the triple point properties are correctly returned
 */
TEST_F(HydrogenFluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 7.7e3, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 13.952, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of vapor pressure, vapor density and saturated liquid
 * density
 */
TEST_F(HydrogenFluidPropertiesTest, vapor)
{
  const Real tol = 10.0 * REL_TOL_EXTERNAL_VALUE;

  // Vapor pressure
  REL_TEST(_fp->vaporPressure(14.0), 7.541e3, tol);
  REL_TEST(_fp->vaporPressure(30.0), 0.80432e6, tol);
}

/**
 * Verify that the coefficients for Henry's constant are correct using
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(HydrogenFluidPropertiesTest, henry)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> hc = _fp->henryCoefficients();

  REL_TEST(hc[0], -4.73284, tol);
  REL_TEST(hc[1], 6.08954, tol);
  REL_TEST(hc[2], 6.06066, tol);
}

/**
 * Verify calculation of thermal conductivity using data from
 * Assael, Assael, Huber, Perkins and Takata, Correlation of te thermal
 * conductivity of normal and parahydrogen from the triple point to 1000 K
 * and up to 100 Mpa, Journal of Physical and Chemical Reference Data, 40 (2011)
 */
TEST_F(HydrogenFluidPropertiesTest, thermalConductivity)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->k_from_rho_T(0.80844, 298.15), 186.97e-3, tol);
  REL_TEST(_fp->k_from_rho_T(30.0, 35.0), 71.854e-3, tol);
  REL_TEST(_fp->k_from_rho_T(15.879, 400.0), 248.6e-3, tol);

  REL_TEST(_fp->k_from_p_T(30.0e6, 400.0), 248.6e-3, tol);
}

/**
 * Verify calculation of viscosity using data from
 * Muzny, Huber and Kazakov, Correlation for the viscosity of normal hydrogen
 * obtained from symbolic regression, Journal of Chemical and Engineering Data,
 * 58, 969-979 (2013)
 */
TEST_F(HydrogenFluidPropertiesTest, viscosity)
{
  const Real tol = 10.0 * REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->mu_from_rho_T(0.0, 298.15), 8.8997e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(0.68911, 350.0), 9.9645e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(6.5764, 350.0), 1.0147e-5, tol);

  REL_TEST(_fp->mu_from_p_T(1.0e6, 350.0), 9.9645e-6, tol);
  REL_TEST(_fp->mu_from_p_T(10.0e6, 350.0), 1.0147e-5, tol);
}

/**
 * Verify calculation of thermophysical properties of Hydrogen using
 * reference data from NIST
 */
TEST_F(HydrogenFluidPropertiesTest, properties)
{
  // Pressure = 1 MPa, temperature = 280 K
  Real p = 1.0e6;
  Real T = 280.0;

  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->rho_from_p_T(p, T), 0.86069, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 3676.1e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 2514.2e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 43.025e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 14.26e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 10.112e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 1283.9, tol);

  // Pressure = 1 MPa, temperature = 500 K
  T = 500.0;
  REL_TEST(_fp->rho_from_p_T(p, T), 0.48302, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 6856.5e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 4786.2e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 51.401e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 14.52e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 10.392e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 1704.1, tol);

  // Pressure = 10 MPa, temperature = 500 K
  p = 10.0e6;
  REL_TEST(_fp->rho_from_p_T(p, T), 4.6658, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 6923.4e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 4780.1e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 41.892e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 14.584e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 10.436e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 1764.3, tol);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(HydrogenFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 1.0e6;
  Real T = 350.0;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);

  // Viscosity from density and temperature
  T = 360.0;
  const Real drho = 1.0e-4;
  Real rho, drho_dp, drho_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  Real dmu_drho_fd =
      (_fp->mu_from_rho_T(rho + drho, T) - _fp->mu_from_rho_T(rho - drho, T)) / (2.0 * drho);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  ABS_TEST(mu, _fp->mu_from_rho_T(rho, T), REL_TOL_CONSISTENCY);
  REL_TEST(dmu_drho, dmu_drho_fd, tol);

  // To properly test derivative wrt temperature, use p and T and calculate density,
  // so that the change in density wrt temperature is included
  const Real dp = 1.0e1;
  const Real dT = 1.0e-4;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  _fp->mu_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  Real dmu_dT_fd = (_fp->mu_from_rho_T(_fp->rho_from_p_T(p, T + dT), T + dT) -
                    _fp->mu_from_rho_T(_fp->rho_from_p_T(p, T - dT), T - dT)) /
                   (2.0 * dT);

  REL_TEST(dmu_dT, dmu_dT_fd, tol);

  Real dmu_dp_fd = (_fp->mu_from_p_T(p + dp, T) - _fp->mu_from_p_T(p - dp, T)) / (2.0 * dp);
  Real dmu_dp = 0.0;
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);

  ABS_TEST(mu, _fp->mu_from_p_T(p, T), REL_TOL_CONSISTENCY);
  REL_TEST(dmu_dp, dmu_dp_fd, tol);

  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  dmu_dT_fd = (_fp->mu_from_p_T(p, T + dT) - _fp->mu_from_p_T(p, T - dT)) / (2.0 * dT);

  REL_TEST(dmu_dT, dmu_dT_fd, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(HydrogenFluidPropertiesTest, combined)
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

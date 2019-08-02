//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NitrogenFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(NitrogenFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "nitrogen"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(NitrogenFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 28.01348e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the critical properties are correctly returned
 */
TEST_F(NitrogenFluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 3.3958e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 126.192, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 313.3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the triple point properties are correctly returned
 */
TEST_F(NitrogenFluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 12.523e3, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 63.151, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of vapor pressure, vapor density and saturated liquid
 * density
 */
TEST_F(NitrogenFluidPropertiesTest, vapor)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  // Vapor pressure
  REL_TEST(_fp->vaporPressure(80.0), 0.13687e6, tol);
  REL_TEST(_fp->vaporPressure(100.0), 0.77827e6, tol);
  REL_TEST(_fp->vaporPressure(120.0), 2.51058e6, tol);
  //
  // Saturated vapor density
  REL_TEST(_fp->saturatedVaporDensity(80.0), 0.21737 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->saturatedVaporDensity(100.0), 1.1409 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->saturatedVaporDensity(120.0), 4.4653 * 1000.0 * _fp->molarMass(), tol);

  // Saturated liquid density
  REL_TEST(_fp->saturatedLiquidDensity(80.0), 28.341 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->saturatedLiquidDensity(100.0), 24.608 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->saturatedLiquidDensity(120.0), 18.682 * 1000.0 * _fp->molarMass(), tol);
}

/**
 * Verify that the coefficients for Henry's constant are correct using
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(NitrogenFluidPropertiesTest, henry)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> hc = _fp->henryCoefficients();

  REL_TEST(hc[0], -9.67578, tol);
  REL_TEST(hc[1], 4.72162, tol);
  REL_TEST(hc[2], 11.70585, tol);
}

/**
 * Verify calculation of thermal conductivity using data from
 * Lemmon and Jacobsen, Viscosity and Thermal Conductivity Equations for Nitrogen,
 * Oxygen, Argon, and Air, International Journal of Thermophysics, 25, 21--69 (2004)
 */
TEST_F(NitrogenFluidPropertiesTest, thermalConductivity)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->k_from_rho_T(0.0, 300.0), 25.9361e-3, tol);
  REL_TEST(_fp->k_from_rho_T(5.0 * 1000.0 * _fp->molarMass(), 300.0), 32.7694e-3, tol);

  const Real p = _fp->p_from_rho_T(5.0 * 1000.0 * _fp->molarMass(), 300.0);
  REL_TEST(_fp->k_from_p_T(p, 300.0), 32.7694e-3, tol);
}

/**
 * Verify calculation of viscosity using data from
 * Lemmon and Jacobsen, Viscosity and Thermal Conductivity Equations for Nitrogen,
 * Oxygen, Argon, and Air, International Journal of Thermophysics, 25, 21--69 (2004)
 */
TEST_F(NitrogenFluidPropertiesTest, viscosity)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->mu_from_rho_T(0.0, 100.0), 6.90349e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(25.0 * 1000.0 * _fp->molarMass(), 100.0), 79.7418e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(10.0 * 1000.0 * _fp->molarMass(), 200.0), 21.081e-6, tol);

  const Real p = _fp->p_from_rho_T(5.0 * 1000.0 * _fp->molarMass(), 300.0);
  REL_TEST(_fp->mu_from_p_T(p, 300.0), 20.7430e-6, tol);
}

/**
 * Verify calculation of thermophysical properties of Nitrogen from the Span et al
 * EOS using verification data provided in "A reference equation of state
 * for the thermodynamic properties of nitrogen for temeperatures from
 * 63.151 to 1000 K and pressures to 2200 MPa", Journal of Physical
 * and Chemical Reference Data, 29, 1361--1433 (2000)
 */
TEST_F(NitrogenFluidPropertiesTest, properties)
{
  // Pressure = 1 MPa, temperature = 280 K
  Real p = 1.0e6;
  Real T = 280.0;

  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->rho_from_p_T(p, T), 0.43104 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->h_from_p_T(p, T), 8070.2 / _fp->molarMass(), tol);
  REL_TEST(_fp->e_from_p_T(p, T), 5750.3 / _fp->molarMass(), tol);
  REL_TEST(_fp->s_from_p_T(p, T), 170.41 / _fp->molarMass(), tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 29.66 / _fp->molarMass(), tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 20.89 / _fp->molarMass(), tol);
  REL_TEST(_fp->c_from_p_T(p, T), 342.4, tol);

  // Pressure = 1 MPa, temperature = 500 K
  T = 500.0;
  REL_TEST(_fp->rho_from_p_T(p, T), 0.23958 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->e_from_p_T(p, T), 10395.0 / _fp->molarMass(), tol);
  REL_TEST(_fp->h_from_p_T(p, T), 14569.0 / _fp->molarMass(), tol);
  REL_TEST(_fp->s_from_p_T(p, T), 187.54 / _fp->molarMass(), tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 29.72 / _fp->molarMass(), tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 21.29 / _fp->molarMass(), tol);
  REL_TEST(_fp->c_from_p_T(p, T), 457.0, tol);

  // Pressure = 10 MPa, temperature = 500 K
  p = 10.0e6;
  REL_TEST(_fp->rho_from_p_T(p, T), 2.3024 * 1000.0 * _fp->molarMass(), tol);
  REL_TEST(_fp->e_from_p_T(p, T), 10154.0 / _fp->molarMass(), tol);
  REL_TEST(_fp->h_from_p_T(p, T), 14497.0 / _fp->molarMass(), tol);
  REL_TEST(_fp->s_from_p_T(p, T), 167.93 / _fp->molarMass(), tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 30.82 / _fp->molarMass(), tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 21.49 / _fp->molarMass(), tol);
  REL_TEST(_fp->c_from_p_T(p, T), 483.1, tol);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(NitrogenFluidPropertiesTest, derivatives)
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
TEST_F(NitrogenFluidPropertiesTest, combined)
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

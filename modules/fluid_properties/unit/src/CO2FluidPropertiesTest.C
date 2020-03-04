//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CO2FluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(CO2FluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "co2"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(CO2FluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 44.0098e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the critical properties are correctly returned
 */
TEST_F(CO2FluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 7.3773e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 304.1282, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 467.6, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the triple point properties are correctly returned
 */
TEST_F(CO2FluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 0.51795e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 216.592, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of melting pressure using experimental data from
 * Michels et al, The melting line of carbon dioxide up to 2800 atmospheres,
 * Physica 9 (1942).
 * Note that results in this reference are given in atm, but have been
 * converted to MPa here.
 * As we are comparing with experimental data, calculated values within 1% are
 * considered satisfactory.
 */
TEST_F(CO2FluidPropertiesTest, melting)
{
  const Real tol = 10.0 * REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->meltingPressure(217.03), 2.57e6, tol);
  REL_TEST(_fp->meltingPressure(235.29), 95.86e6, tol);
  REL_TEST(_fp->meltingPressure(266.04), 286.77e6, tol);
}

/**
 * Verify calculation of sublimation pressure using data from
 * Bedford et al., Recommended values of temperature for a selected set of
 * secondary reference points, Metrologia 20 (1984).
 */
TEST_F(CO2FluidPropertiesTest, sublimation)
{
  REL_TEST(_fp->sublimationPressure(194.6857), 0.101325e6, REL_TOL_EXTERNAL_VALUE);
}

/**
 * Verify calculation of vapor pressure, vapor density and saturated liquid
 * density using experimental data from
 * Duschek et al., Measurement and correlation of the (pressure, density, temperature)
 * relation of cabon dioxide II. Saturated-liquid and saturated-vapor densities and
 * the vapor pressure along the entire coexstance curve, J. Chem. Thermo. 22 (1990).
 * As we are comparing with experimental data, calculated values within 1% are
 * considered satisfactory.
 */
TEST_F(CO2FluidPropertiesTest, vapor)
{
  const Real tol = 10.0 * REL_TOL_EXTERNAL_VALUE;

  // Vapor pressure
  REL_TEST(_fp->vaporPressure(217.0), 0.52747e6, tol);
  REL_TEST(_fp->vaporPressure(245.0), 1.51887e6, tol);
  REL_TEST(_fp->vaporPressure(303.8), 7.32029e6, tol);

  // Saturated vapor density
  REL_TEST(_fp->saturatedVaporDensity(217.0), 14.0017, tol);
  REL_TEST(_fp->saturatedVaporDensity(245.0), 39.5048, tol);
  REL_TEST(_fp->saturatedVaporDensity(303.8), 382.30, tol);

  // Saturated liquid density
  REL_TEST(_fp->saturatedLiquidDensity(217.0), 1177.03, tol);
  REL_TEST(_fp->saturatedLiquidDensity(245.0), 1067.89, tol);
  REL_TEST(_fp->saturatedLiquidDensity(303.8), 554.14, tol);
}

/**
 * Verify calculation of partial density at infinite dilution using data from
 * Hnedkovsky et al., Volumes of aqueous solutions of CH4, CO2, H2S, and NH3
 * at temperatures from 298.15 K to 705 K and pressures to 35 MPa,
 * J. Chem. Thermo. 28, 1996.
 * As we are comparing with experimental data, calculated values within 5% are
 * considered satisfactory.
 */
TEST_F(CO2FluidPropertiesTest, partialDensity)
{
  const Real tol = 50.0 * REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->partialDensity(373.15), 1182.8, tol);
  REL_TEST(_fp->partialDensity(473.35), 880.0, tol);
  REL_TEST(_fp->partialDensity(573.15), 593.8, tol);
}

/**
 * Verify that the coefficients for Henry's constant are correct using
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(CO2FluidPropertiesTest, henry)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> hc = _fp->henryCoefficients();

  REL_TEST(hc[0], -8.55445, tol);
  REL_TEST(hc[1], 4.01195, tol);
  REL_TEST(hc[2], 9.52345, tol);
}

/**
 * Verify calculation of thermal conductivity using data from
 * Scalabrin et al., A Reference Multiparameter Thermal Conductivity Equation
 * for Carbon Dioxide with an Optimized Functional Form,
 * J. Phys. Chem. Ref. Data 35 (2006)
 */
TEST_F(CO2FluidPropertiesTest, thermalConductivity)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->k_from_rho_T(23.435, 250.0), 13.45e-3, tol);
  REL_TEST(_fp->k_from_rho_T(18.579, 300.0), 17.248e-3, tol);
  REL_TEST(_fp->k_from_rho_T(11.899, 450.0), 29.377e-3, tol);

  REL_TEST(_fp->k_from_p_T(1.0e6, 250.0), 1.34504e-2, tol);
  REL_TEST(_fp->k_from_p_T(1.0e6, 300.0), 1.72483e-2, tol);
  REL_TEST(_fp->k_from_p_T(1.0e6, 450.0), 2.93767e-2, tol);
}

/**
 * Verify calculation of viscosity using data from
 * Fenghour et al., The viscosity of carbon dioxide,
 * J. Phys. Chem. Ref. Data, 27, 31-44 (1998)
 */
TEST_F(CO2FluidPropertiesTest, viscosity)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->mu_from_rho_T(20.199, 280.0), 14.15e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(15.105, 360.0), 17.94e-6, tol);
  REL_TEST(_fp->mu_from_rho_T(10.664, 500.0), 24.06e-6, tol);

  REL_TEST(_fp->mu_from_p_T(1.0e6, 280.0), 1.41505e-05, tol);
  REL_TEST(_fp->mu_from_p_T(1.0e6, 360.0), 1.79395e-05, tol);
  REL_TEST(_fp->mu_from_p_T(1.0e6, 500.0), 2.40643e-05, tol);
}

/**
 * Verify calculation of thermophysical properties of CO2 from the Span and
 * Wagner EOS using verification data provided in
 * A New Equation of State for Carbon Dioxide Covering the Fluid Region from
 * the Triple-Point Temperature to 1100K at Pressures up to 800 MPa,
 * J. Phys. Chem. Ref. Data, 25 (1996)
 */
TEST_F(CO2FluidPropertiesTest, propertiesSW)
{
  // Pressure = 1 MPa, temperature = 280 K
  Real p = 1.0e6;
  Real T = 280.0;

  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->rho_from_p_T(p, T), 20.199, tol);
  REL_TEST(_fp->h_from_p_T(p, T), -26.385e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), -75.892e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), -0.51326e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 0.92518e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 0.67092e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 252.33, tol);

  // Pressure = 1 MPa, temperature = 500 K
  T = 500.0;
  REL_TEST(_fp->rho_from_p_T(p, T), 10.664, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 185.60e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 91.829e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 0.04225e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 1.0273e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 0.82823e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 339.81, tol);

  // Pressure = 10 MPa, temperature = 500 K
  p = 10.0e6;
  REL_TEST(_fp->rho_from_p_T(p, T), 113.07, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 157.01e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 68.569e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), -0.4383e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 1.1624e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 0.85516e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 337.45, tol);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(CO2FluidPropertiesTest, derivatives)
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
TEST_F(CO2FluidPropertiesTest, combined)
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

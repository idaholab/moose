//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BrineFluidPropertiesTest.h"
#include "FluidPropertiesTestUtils.h"

#include "Water97FluidProperties.h"
#include "NaClFluidProperties.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(BrineFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "brine"); }

/**
 * Test that the molar masses are correctly returned
 */
TEST_F(BrineFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMassH2O(), 18.015e-3, REL_TOL_CONSISTENCY);
  ABS_TEST(_fp->molarMassNaCl(), 58.443e-3, REL_TOL_CONSISTENCY);

  // Molar mass of water with salt mass fraction 0.1
  const Real x = 0.1;
  const Real M = 1.0 / (x / _fp->molarMassNaCl() + (1.0 - x) / _fp->molarMassH2O());
  ABS_TEST(_fp->molarMass(x), M, REL_TOL_CONSISTENCY);
}

/**
 * Test that the correct fluid component userobject is returned
 */
TEST_F(BrineFluidPropertiesTest, getComponent)
{
  auto & water_fp = _fp->getComponent(BrineFluidProperties::WATER);
  auto & nacl_fp = _fp->getComponent(BrineFluidProperties::NACL);

  EXPECT_EQ(water_fp.fluidName(), "water");
  EXPECT_EQ(nacl_fp.fluidName(), "nacl");
}

/**
 * Verify calculation of brine vapor pressure using data from
 * Haas, Physical properties of the coexisting phases and thermochemical
 * properties of the H2O component in boiling NaCl solutions, Geological Survey
 * Bulletin, 1421-A (1976).
 */
TEST_F(BrineFluidPropertiesTest, vapor)
{
  REL_TEST(_fp->vaporPressure(473.15, 0.185), 1.34e6, 1.0e-2);
  REL_TEST(_fp->vaporPressure(473.15, 0.267), 1.21e6, 1.0e-2);
  REL_TEST(_fp->vaporPressure(473.15, 0.312), 1.13e6, 1.0e-2);
}

/**
 * Verify calculation of halite solubility using data from
 * Bodnar et al, Synthetic fluid inclusions in natural quartz, III.
 * Determination of phase equilibrium properties in the system H2O-NaCl
 * to 1000C and 1500 bars, Geocehmica et Cosmochemica Acta, 49, 1861-1873 (1985).
 * Note that the average of the range quoted has been used for each point.
 */
TEST_F(BrineFluidPropertiesTest, solubility)
{
  REL_TEST(_fp->haliteSolubility(659.65), 0.442, 2.0e-2);
  REL_TEST(_fp->haliteSolubility(818.65), 0.6085, 2.0e-2);
  REL_TEST(_fp->haliteSolubility(903.15), 0.7185, 2.0e-2);
}

/**
 * Verify calculation of brine properties.
 * Experimental density values from Pitzer et al, Thermodynamic properties
 * of aqueous sodium chloride solution, Journal of Physical and Chemical
 * Reference Data, 13, 1-102 (1984)
 *
 * Experimental viscosity values from Phillips et al, Viscosity of NaCl and
 * other solutions up to 350C and 50MPa pressures, LBL-11586 (1980)
 *
 * Thermal conductivity values from Ozbek and Phillips, Thermal conductivity of
 * aqueous NaCl solutions from 20C to 330C, LBL-9086 (1980)
 *
 *  It is difficult to compare enthalpy and cp with experimental data, so
 * instead we recreate the data presented in Figures 11 and 12 of
 * Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
 * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar,
 * and 0 to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007)
 */
TEST_F(BrineFluidPropertiesTest, properties)
{
  // Pressure, temperature and NaCl mass fraction for tests
  Real p0 = 20.0e6;
  Real p1 = 40.0e6;
  Real T0 = 323.15;
  Real T1 = 473.15;
  Real x0 = 0.1047;
  Real x1 = 0.2261;

  // Density
  REL_TEST(_fp->rho_from_p_T_X(p0, T0, x0), 1068.52, 1.0e-2);
  REL_TEST(_fp->rho_from_p_T_X(p0, T1, x0), 959.27, 1.0e-2);
  REL_TEST(_fp->rho_from_p_T_X(p1, T1, x1), 1065.58, 1.0e-2);

  // Viscosity
  REL_TEST(_fp->mu_from_p_T_X(p0, T0, x0), 679.8e-6, 2.0e-2);
  REL_TEST(_fp->mu_from_p_T_X(p0, T1, x0), 180.0e-6, 2.0e-2);
  REL_TEST(_fp->mu_from_p_T_X(p1, T1, x1), 263.1e-6, 2.0e-2);

  // Thermal conductivity
  REL_TEST(_fp->k_from_p_T_X(p0, T0, x0), 0.630, 4.0e-2);
  REL_TEST(_fp->k_from_p_T_X(p0, T1, x0), 0.649, 4.0e-2);
  REL_TEST(_fp->k_from_p_T_X(p1, T1, x1), 0.633, 4.0e-2);

  // Enthalpy
  p0 = 10.0e6;
  T0 = 573.15;

  REL_TEST(_fp->e_from_p_T_X(p0, T0, 0.0), 1330.0e3, 1.0e-2);
  REL_TEST(_fp->e_from_p_T_X(p0, T0, 0.2), 1100.0e3, 1.0e-2);
  REL_TEST(_fp->e_from_p_T_X(p0, T0, 0.364), 970.0e3, 1.0e-2);

  // cp
  p0 = 17.9e6;
  x0 = 0.01226;

  REL_TEST(_fp->cp_from_p_T_X(p0, 323.15, x0), 4.1e3, 1.0e-2);
  REL_TEST(_fp->cp_from_p_T_X(p0, 473.15, x0), 4.35e3, 1.0e-2);
  REL_TEST(_fp->cp_from_p_T_X(p0, 623.15, x0), 8.1e3, 1.0e-2);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(BrineFluidPropertiesTest, derivatives)
{
  Real p = 1.0e6;
  Real T = 350.0;
  Real x = 0.1047;

  // Finite differencing parameters
  Real dp = 1.0e-2;
  Real dT = 1.0e-4;
  Real dx = 1.0e-8;

  // Density
  Real drho_dp_fd =
      (_fp->rho_from_p_T_X(p + dp, T, x) - _fp->rho_from_p_T_X(p - dp, T, x)) / (2.0 * dp);
  Real drho_dT_fd =
      (_fp->rho_from_p_T_X(p, T + dT, x) - _fp->rho_from_p_T_X(p, T - dT, x)) / (2.0 * dT);
  Real drho_dx_fd =
      (_fp->rho_from_p_T_X(p, T, x + dx) - _fp->rho_from_p_T_X(p, T, x - dx)) / (2.0 * dx);

  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0, drho_dx = 0.0;
  _fp->rho_from_p_T_X(p, T, x, rho, drho_dp, drho_dT, drho_dx);

  ABS_TEST(rho, _fp->rho_from_p_T_X(p, T, x), REL_TOL_CONSISTENCY);
  REL_TEST(drho_dp, drho_dp_fd, 1.0e-5);
  REL_TEST(drho_dT, drho_dT_fd, 1.0e-6);
  REL_TEST(drho_dx, drho_dx_fd, 1.0e-6);

  // Enthalpy
  Real dh_dp_fd = (_fp->h_from_p_T_X(p + dp, T, x) - _fp->h_from_p_T_X(p - dp, T, x)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h_from_p_T_X(p, T + dT, x) - _fp->h_from_p_T_X(p, T - dT, x)) / (2.0 * dT);
  Real dh_dx_fd = (_fp->h_from_p_T_X(p, T, x + dx) - _fp->h_from_p_T_X(p, T, x - dx)) / (2.0 * dx);

  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0, dh_dx = 0.0;
  _fp->h_from_p_T_X(p, T, x, h, dh_dp, dh_dT, dh_dx);

  ABS_TEST(h, _fp->h_from_p_T_X(p, T, x), REL_TOL_CONSISTENCY);
  REL_TEST(dh_dp, dh_dp_fd, 1.0e-4);
  REL_TEST(dh_dT, dh_dT_fd, 1.0e-6);
  REL_TEST(dh_dx, dh_dx_fd, 1.0e-6);

  // Internal energy
  Real de_dp_fd = (_fp->e_from_p_T_X(p + dp, T, x) - _fp->e_from_p_T_X(p - dp, T, x)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e_from_p_T_X(p, T + dT, x) - _fp->e_from_p_T_X(p, T - dT, x)) / (2.0 * dT);
  Real de_dx_fd = (_fp->e_from_p_T_X(p, T, x + dx) - _fp->e_from_p_T_X(p, T, x - dx)) / (2.0 * dx);

  Real e = 0.0, de_dp = 0.0, de_dT = 0.0, de_dx = 0.0;
  _fp->e_from_p_T_X(p, T, x, e, de_dp, de_dT, de_dx);

  ABS_TEST(e, _fp->e_from_p_T_X(p, T, x), REL_TOL_CONSISTENCY);
  REL_TEST(de_dp, de_dp_fd, 1.0e-3);
  REL_TEST(de_dT, de_dT_fd, 1.0e-6);
  REL_TEST(de_dx, de_dx_fd, 1.0e-6);

  // Viscosity
  Real dmu_dp_fd =
      (_fp->mu_from_p_T_X(p + dp, T, x) - _fp->mu_from_p_T_X(p - dp, T, x)) / (2.0 * dp);
  Real dmu_dT_fd =
      (_fp->mu_from_p_T_X(p, T + dT, x) - _fp->mu_from_p_T_X(p, T - dT, x)) / (2.0 * dT);
  Real dmu_dx_fd =
      (_fp->mu_from_p_T_X(p, T, x + dx) - _fp->mu_from_p_T_X(p, T, x - dx)) / (2.0 * dx);
  Real mu = 0.0, dmu_dp = 0.0, dmu_dT = 0.0, dmu_dx = 0.0;
  _fp->mu_from_p_T_X(p, T, x, mu, dmu_dp, dmu_dT, dmu_dx);

  ABS_TEST(mu, _fp->mu_from_p_T_X(p, T, x), REL_TOL_CONSISTENCY);
  REL_TEST(dmu_dp, dmu_dp_fd, 1.0e-3);
  REL_TEST(dmu_dT, dmu_dT_fd, 1.0e-6);
  REL_TEST(dmu_dx, dmu_dx_fd, 1.0e-6);

  // Verify that derivatives wrt x are defined when x = 0
  x = 0.0;

  // Density
  _fp->rho_from_p_T_X(p, T, x, rho, drho_dp, drho_dT, drho_dx);
  drho_dx_fd = (_fp->rho_from_p_T_X(p, T, x + dx) - _fp->rho_from_p_T_X(p, T, x)) / dx;

  REL_TEST(drho_dx, drho_dx_fd, 1.0e-3);

  // Enthalpy
  _fp->h_from_p_T_X(p, T, x, h, dh_dp, dh_dT, dh_dx);
  dh_dx_fd = (_fp->h_from_p_T_X(p, T, x + dx) - _fp->h_from_p_T_X(p, T, x)) / dx;

  REL_TEST(dh_dx, dh_dx_fd, 1.0e-3);

  // Internal energy
  _fp->e_from_p_T_X(p, T, x, e, de_dp, de_dT, de_dx);
  de_dx_fd = (_fp->e_from_p_T_X(p, T, x + dx) - _fp->e_from_p_T_X(p, T, x)) / dx;

  REL_TEST(de_dx, de_dx_fd, 1.0e-3);

  // Viscosity
  dmu_dx_fd = (_fp->mu_from_p_T_X(p, T, x + dx) - _fp->mu_from_p_T_X(p, T, x)) / dx;
  _fp->mu_from_p_T_X(p, T, x, mu, dmu_dp, dmu_dT, dmu_dx);

  REL_TEST(dmu_dx, dmu_dx_fd, 1.0e-3);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(BrineFluidPropertiesTest, combined)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real x = 0.1047;

  // Single property methods
  Real rho, drho_dp, drho_dT, drho_dx, mu, dmu_dp, dmu_dT, dmu_dx;
  _fp->rho_from_p_T_X(p, T, x, rho, drho_dp, drho_dT, drho_dx);
  _fp->mu_from_p_T_X(p, T, x, mu, dmu_dp, dmu_dT, dmu_dx);

  // Combined property methods
  Real rho2, mu2;
  _fp->rho_mu_from_p_T_X(p, T, x, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  // Combined property method with derivatives
  Real drho2_dp, drho2_dT, drho2_dx, dmu2_dp, dmu2_dT, dmu2_dx;
  _fp->rho_mu_from_p_T_X(
      p, T, x, rho2, drho2_dp, drho2_dT, drho2_dx, mu2, dmu2_dp, dmu2_dT, dmu2_dx);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(drho_dx, drho2_dx, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);
  ABS_TEST(dmu_dx, dmu2_dx, tol);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CO2FluidPropertiesTest.h"
#include "Utils.h"

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
  REL_TEST("melting", _fp->meltingPressure(217.03), 2.57e6, 1.0e-2);
  REL_TEST("melting", _fp->meltingPressure(235.29), 95.86e6, 1.0e-2);
  REL_TEST("melting", _fp->meltingPressure(266.04), 286.77e6, 1.0e-2);
}

/**
 * Verify calculation of sublimation pressure using data from
 * Bedford et al., Recommended values of temperature for a selected set of
 * secondary reference points, Metrologia 20 (1984).
 */
TEST_F(CO2FluidPropertiesTest, sublimation)
{
  REL_TEST("sublimation", _fp->sublimationPressure(194.6857), 0.101325e6, 1.0e-4);
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
  // Vapor pressure
  REL_TEST("vapor", _fp->vaporPressure(217.0), 0.52747e6, 1.0e-2);
  REL_TEST("vapor", _fp->vaporPressure(245.0), 1.51887e6, 1.0e-2);
  REL_TEST("vapor", _fp->vaporPressure(303.8), 7.32029e6, 1.0e-2);

  // Saturated vapor density
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(217.0), 14.0017, 1.0e-2);
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(245.0), 39.5048, 1.0e-2);
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(303.8), 382.30, 1.0e-2);

  // Saturated liquid density
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(217.0), 1177.03, 1.0e-2);
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(245.0), 1067.89, 1.0e-2);
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(303.8), 554.14, 1.0e-2);
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
  REL_TEST("partial density", _fp->partialDensity(373.15), 1182.8, 5.0e-2);
  REL_TEST("partial density", _fp->partialDensity(473.35), 880.0, 5.0e-2);
  REL_TEST("partial density", _fp->partialDensity(573.15), 593.8, 5.0e-2);
}

/**
 * Verify calculation of Henry's constant using data from
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(CO2FluidPropertiesTest, henry)
{
  REL_TEST("henry", _fp->henryConstant(300.0), 173.63e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(400.0), 579.84e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(500.0), 520.79e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(600.0), 259.53e6, 1.0e-3);
}

/**
 * Verify calculation of thermal conductivity using data from
 * Scalabrin et al., A Reference Multiparameter Thermal Conductivity Equation
 * for Carbon Dioxide with an Optimized Functional Form,
 * J. Phys. Chem. Ref. Data 35 (2006)
 */
TEST_F(CO2FluidPropertiesTest, thermalConductivity)
{
  REL_TEST("thermal conductivity", _fp->k_from_rho_T(23.435, 250.0), 13.45e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k_from_rho_T(18.579, 300.0), 17.248e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k_from_rho_T(11.899, 450.0), 29.377e-3, 1.0e-3);
}

TEST_F(CO2FluidPropertiesTest, thermalConductivity2)
{
  REL_TEST("thermal conductivity", _fp->k(1.0e6, 250.0), 1.34504e-2, 1.0e-6);
  REL_TEST("thermal conductivity", _fp->k(1.0e6, 300.0), 1.72483e-2, 3.0e-6);
  REL_TEST("thermal conductivity", _fp->k(1.0e6, 450.0), 2.93767e-2, 1.0e-6);
}

/**
 * Verify calculation of viscosity using data from
 * Fenghour et al., The viscosity of carbon dioxide,
 * J. Phys. Chem. Ref. Data, 27, 31-44 (1998)
 */
TEST_F(CO2FluidPropertiesTest, viscosity)
{
  REL_TEST("viscosity", _fp->mu_from_rho_T(20.199, 280.0), 14.15e-6, 1.0e-3);
  REL_TEST("viscosity", _fp->mu_from_rho_T(15.105, 360.0), 17.94e-6, 1.0e-3);
  REL_TEST("viscosity", _fp->mu_from_rho_T(10.664, 500.0), 24.06e-6, 1.0e-3);
}

TEST_F(CO2FluidPropertiesTest, viscosity2)
{
  REL_TEST("viscosity", _fp->mu(1.0e6, 280.0), 1.41505e-05, 3.0e-6);
  REL_TEST("viscosity", _fp->mu(1.0e6, 360.0), 1.79395e-05, 2.0e-6);
  REL_TEST("viscosity", _fp->mu(1.0e6, 500.0), 2.40643e-05, 1.0e-6);
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
  REL_TEST("density", _fp->rho(p, T), 20.199, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), -26.385e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), -75.892e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), -0.51326e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 0.92518e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.67092e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 252.33, 1.0e-3);

  // Pressure = 1 MPa, temperature = 500 K
  T = 500.0;
  REL_TEST("density", _fp->rho(p, T), 10.664, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 185.60e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 91.829e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), 0.04225e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 1.0273e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.82823e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 339.81, 1.0e-3);

  // Pressure = 10 MPa, temperature = 500 K
  p = 10.0e6;
  REL_TEST("density", _fp->rho(p, T), 113.07, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 157.01e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 68.569e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), -0.4383e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 1.1624e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.85516e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 337.45, 1.0e-3);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(CO2FluidPropertiesTest, derivatives)
{
  Real p = 1.0e6;
  Real T = 350.0;

  // Finite differencing parameters
  Real dp = 1.0e1;
  Real dT = 1.0e-4;

  // density
  Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, 1.0e-6);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, 1.0e-6);

  // enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);

  ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
  REL_TEST("dh_dp", dh_dp, dh_dp_fd, 1.0e-6);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, 1.0e-6);

  // internal energy
  Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
  _fp->e_dpT(p, T, e, de_dp, de_dT);

  ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
  REL_TEST("de_dp", de_dp, de_dp_fd, 1.0e-6);
  REL_TEST("de_dT", de_dT, de_dT_fd, 1.0e-6);

  // Viscosity
  rho = 15.105;
  T = 360.0;
  Real drho = 1.0e-4;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  Real dmu_drho_fd =
      (_fp->mu_from_rho_T(rho + drho, T) - _fp->mu_from_rho_T(rho - drho, T)) / (2.0 * drho);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu_from_rho_T(rho, T), 1.0e-15);
  REL_TEST("dmu_drho", dmu_drho, dmu_drho_fd, 1.0e-6);

  // To properly test derivative wrt temperature, use p and T and calculate density,
  // so that the change in density wrt temperature is included
  p = 1.0e6;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);
  _fp->mu_drhoT_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);
  Real dmu_dT_fd = (_fp->mu_from_rho_T(_fp->rho(p, T + dT), T + dT) -
                    _fp->mu_from_rho_T(_fp->rho(p, T - dT), T - dT)) /
                   (2.0 * dT);

  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  Real dmu_dp_fd = (_fp->mu(p + dp, T) - _fp->mu(p - dp, T)) / (2.0 * dp);
  Real dmu_dp = 0.0;
  _fp->mu_dpT(p, T, mu, dmu_dp, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu(p, T), 1.0e-15);
  REL_TEST("dmu_dp", dmu_dp, dmu_dp_fd, 1.0e-6);

  _fp->mu_dpT(p, T, mu, dmu_dp, dmu_dT);
  dmu_dT_fd = (_fp->mu(p, T + dT) - _fp->mu(p, T - dT)) / (2.0 * dT);

  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  // Henry's constant
  T = 300.0;

  Real dKh_dT_fd = (_fp->henryConstant(T + dT) - _fp->henryConstant(T - dT)) / (2.0 * dT);
  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant_dT(T, Kh, dKh_dT);
  REL_TEST("henry", Kh, _fp->henryConstant(T), 1.0e-6);
  REL_TEST("dhenry_dT", dKh_dT_fd, dKh_dT, 1.0e-6);
}

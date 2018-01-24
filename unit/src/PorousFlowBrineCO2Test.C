//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrineCO2Test.h"

/**
 * Verify that the correct name is supplied
 */
TEST_F(PorousFlowBrineCO2Test, name) { EXPECT_EQ("brine-co2", _fp->fluidStateName()); }

/*
 * Verify calculation of the equilibrium constants and their derivatives wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumConstants)
{
  const Real T = 350.0;
  const Real dT = 1.0e-6;

  Real K0H2O, dK0H2O_dT, K0CO2, dK0CO2_dT;
  _fp->equilibriumConstantH2O(T, K0H2O, dK0H2O_dT);
  _fp->equilibriumConstantCO2(T, K0CO2, dK0CO2_dT);

  ABS_TEST("K0H2O", K0H2O, 0.412597711705, 1.0e-10);
  ABS_TEST("K0CO2", K0CO2, 74.0435888596, 1.0e-10);

  Real K0H2O_2, dK0H2O_2_dT, K0CO2_2, dK0CO2_2_dT;
  _fp->equilibriumConstantH2O(T + dT, K0H2O_2, dK0H2O_2_dT);
  _fp->equilibriumConstantCO2(T + dT, K0CO2_2, dK0CO2_2_dT);

  Real dK0H2O_dT_fd = (K0H2O_2 - K0H2O) / dT;
  Real dK0CO2_dT_fd = (K0CO2_2 - K0CO2) / dT;
  REL_TEST("dK0H2O_dT", dK0H2O_dT, dK0H2O_dT_fd, 1.0e-6);
  REL_TEST("dK0CO2_dT", dK0CO2_dT, dK0CO2_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of the fugacity coefficients and their derivatives wrt
 * pressure and temperature
 */
TEST_F(PorousFlowBrineCO2Test, fugacityCoefficients)
{
  const Real p = 40.0e6;
  const Real T = 350.0;
  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;

  Real phiH2O, dphiH2O_dp, dphiH2O_dT;
  Real phiCO2, dphiCO2_dp, dphiCO2_dT;
  _fp->fugacityCoefficientCO2(p, T, phiCO2, dphiCO2_dp, dphiCO2_dT);
  _fp->fugacityCoefficientH2O(p, T, phiH2O, dphiH2O_dp, dphiH2O_dT);

  ABS_TEST("phiCO2 ", phiCO2, 0.40, 1.0e-2);
  ABS_TEST("phiH2O ", phiH2O, 0.09, 1.0e-2);

  Real phiH2O_2, dphiH2O_2_dp, dphiH2O_2_dT;
  Real phiCO2_2, dphiCO2_2_dp, dphiCO2_2_dT;
  _fp->fugacityCoefficientCO2(p + dp, T, phiCO2_2, dphiCO2_2_dp, dphiCO2_2_dT);
  _fp->fugacityCoefficientH2O(p + dp, T, phiH2O_2, dphiH2O_2_dp, dphiH2O_2_dT);

  Real dphiH2O_dp_fd = (phiH2O_2 - phiH2O) / dp;
  Real dphiCO2_dp_fd = (phiCO2_2 - phiCO2) / dp;
  REL_TEST("dphiH2O_dp", dphiH2O_dp, dphiH2O_dp_fd, 1.0e-2);
  REL_TEST("dphiCO2_dp", dphiCO2_dp, dphiCO2_dp_fd, 1.0e-2);

  _fp->fugacityCoefficientCO2(p, T + dT, phiCO2_2, dphiCO2_2_dp, dphiCO2_2_dT);
  _fp->fugacityCoefficientH2O(p, T + dT, phiH2O_2, dphiH2O_2_dp, dphiH2O_2_dT);

  Real dphiH2O_dT_fd = (phiH2O_2 - phiH2O) / dT;
  Real dphiCO2_dT_fd = (phiCO2_2 - phiCO2) / dT;
  REL_TEST("dphiH2O_dT", dphiH2O_dT, dphiH2O_dT_fd, 1.0e-2);
  REL_TEST("dphiCO2_dT", dphiCO2_dT, dphiCO2_dT_fd, 1.0e-2);
}

/*
 * Verify calculation of the activity coefficient and its derivatives wrt
 * pressure and temperature
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficient)
{
  const Real p = 10.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;
  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;

  Real gamma, dgamma_dp, dgamma_dT;
  _fp->activityCoefficient(p, T, xnacl, gamma, dgamma_dp, dgamma_dT);
  ABS_TEST("gamma", gamma, 1.43, 1.0e-2);

  Real gamma_2, dgamma_2_dp, dgamma_2_dT;
  _fp->activityCoefficient(p + dp, T, xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT);

  Real dgamma_dp_fd = (gamma_2 - gamma) / dp;
  REL_TEST("dgamma_dp", dgamma_dp, dgamma_dp_fd, 1.0e-6);

  _fp->activityCoefficient(p, T + dT, xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT);

  Real dgamma_dT_fd = (gamma_2 - gamma) / dT;
  REL_TEST("dgamma_dT", dgamma_dT, dgamma_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of the partial density of CO2 and its derivative wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, partialDensity)
{
  const Real T = 473.15;
  const Real dT = 1.0e-6;

  Real partial_density, dpartial_density_dT;
  _fp->partialDensityCO2(T, partial_density, dpartial_density_dT);
  ABS_TEST("partialDensity", partial_density, 893.332, 1.0e-3);

  Real partial_density_2, dpartial_density_2_dT;
  _fp->partialDensityCO2(T + dT, partial_density_2, dpartial_density_2_dT);

  Real dpartial_density_dT_fd = (partial_density_2 - partial_density) / dT;
  REL_TEST("dpartial_density_dT", dpartial_density_dT, dpartial_density_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of equilibrium mass fraction and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;

  Real Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  Real Xco21, dXco21_dp, dXco21_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT;
  Real Xco22, dXco22_dp, dXco22_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT;
  _fp->equilibriumMassFractions(p, T, xnacl, Xco2, dXco2_dp, dXco2_dT, Yh2o, dYh2o_dp, dYh2o_dT);
  _fp->equilibriumMassFractions(
      p - dp, T, xnacl, Xco21, dXco21_dp, dXco21_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT);
  _fp->equilibriumMassFractions(
      p + dp, T, xnacl, Xco22, dXco22_dp, dXco22_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT);

  Real dXco2_dp_fd = (Xco22 - Xco21) / (2.0 * dp);
  Real dYh2o_dp_fd = (Yh2o2 - Yh2o1) / (2.0 * dp);

  REL_TEST("dXco2_dp", dXco2_dp, dXco2_dp_fd, 1.0e-6);
  REL_TEST("dYh2o_dp", dYh2o_dp, dYh2o_dp_fd, 1.0e-6);

  _fp->equilibriumMassFractions(
      p, T - dT, xnacl, Xco21, dXco21_dp, dXco21_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT);
  _fp->equilibriumMassFractions(
      p, T + dT, xnacl, Xco22, dXco22_dp, dXco22_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT);

  Real dXco2_dT_fd = (Xco22 - Xco21) / (2.0 * dT);
  Real dYh2o_dT_fd = (Yh2o2 - Yh2o1) / (2.0 * dT);

  REL_TEST("dXco2_dT", dXco2_dT, dXco2_dT_fd, 1.0e-6);
  REL_TEST("dYh2o_dT", dYh2o_dT, dYh2o_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of actual mass fraction and derivatives depending on value of
 * total mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, MassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Liquid region
  Real z = 0.0001;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  Real Xco2 = fsp[0].mass_fraction[1];
  Real Yco2 = fsp[1].mass_fraction[1];
  Real Xh2o = fsp[0].mass_fraction[0];
  Real Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xco2", Xco2, z, 1.0e-8);
  ABS_TEST("Yco2", Yco2, 0.0, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - z, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 0.0, 1.0e-8);

  // Verify derivatives
  Real dXco2_dp = fsp[0].dmass_fraction_dp[1];
  Real dXco2_dT = fsp[0].dmass_fraction_dT[1];
  Real dXco2_dz = fsp[0].dmass_fraction_dz[1];
  Real dYco2_dp = fsp[1].dmass_fraction_dp[1];
  Real dYco2_dT = fsp[1].dmass_fraction_dT[1];
  Real dYco2_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXco2_dp", dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 1.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dz", dYco2_dz, 0.0, 1.0e-8);

  // Gas region
  z = 0.995;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xco2", Xco2, 0.0, 1.0e-8);
  ABS_TEST("Yco2", Yco2, z, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 0.0, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 1.0 - z, 1.0e-8);

  // Verify derivatives
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dz = fsp[0].dmass_fraction_dz[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXco2_dp", dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dz", dYco2_dz, 1.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  //  be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  z = 0.45;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  Real Xco2_eq, dXco2_dp_eq, dXco2_dT_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq;
  _fp->equilibriumMassFractions(
      p, T, xnacl, Xco2_eq, dXco2_dp_eq, dXco2_dT_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xco2", Xco2, Xco2_eq, 1.0e-8);
  ABS_TEST("Yco2", Yco2, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - Xco2_eq, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, Yh2o_eq, 1.0e-8);

  // Verify derivatives wrt p and T
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dz = fsp[0].dmass_fraction_dz[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXco2_dp", dXco2_dp, dXco2_dp_eq, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, dXco2_dT_eq, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, -dYh2o_dp_eq, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, -dYh2o_dT_eq, 1.0e-8);
  ABS_TEST("dYco2_dz", dYco2_dz, 0.0, 1.0e-8);

  // Use finite differences to verify derivative wrt z is unaffected by z
  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  Real Xco21 = fsp[0].mass_fraction[1];
  Real Yco21 = fsp[1].mass_fraction[1];
  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  Real Xco22 = fsp[0].mass_fraction[1];
  Real Yco22 = fsp[1].mass_fraction[1];

  ABS_TEST("dXco2_dz", dXco2_dz, (Xco21 - Xco22) / (2.0 * dz), 1.0e-8);
  ABS_TEST("dXYco2_dz", dYco2_dz, (Yco21 - Yco22) / (2.0 * dz), 1.0e-8);
}

/*
 * Verify calculation of gas density and viscosity, and derivatives. Note that as
 * these properties don't depend on mass fraction, only the gas region needs to be
 * tested (the calculations are identical in the two phase region)
 */
TEST_F(PorousFlowBrineCO2Test, gasProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Gas region
  Real z = 0.995;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density and viscosity
  _fp->gasProperties(p, T, fsp);
  Real gas_density = fsp[1].density;
  Real gas_viscosity = fsp[1].viscosity;
  Real density = _co2_fp->rho(p, T);
  Real viscosity = _co2_fp->mu(p, T);

  ABS_TEST("gas density", gas_density, density, 1.0e-8);
  ABS_TEST("gas viscosity", gas_viscosity, viscosity, 1.0e-8);

  // Verify derivatives
  Real ddensity_dp = fsp[1].ddensity_dp;
  Real ddensity_dT = fsp[1].ddensity_dT;
  Real ddensity_dz = fsp[1].ddensity_dz;
  Real dviscosity_dp = fsp[1].dviscosity_dp;
  Real dviscosity_dT = fsp[1].dviscosity_dT;
  Real dviscosity_dz = fsp[1].dviscosity_dz;

  const Real dp = 1.0e-1;
  _fp->gasProperties(p + dp, T, fsp);
  Real rho1 = fsp[1].density;
  Real mu1 = fsp[1].viscosity;

  _fp->gasProperties(p - dp, T, fsp);
  Real rho2 = fsp[1].density;
  Real mu2 = fsp[1].viscosity;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);

  const Real dT = 1.0e-3;
  _fp->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  _fp->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);

  // Note: mass fraction changes with z
  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  ABS_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-8);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-8);
}

/*
 * Verify calculation of liquid density and viscosity, and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, liquidProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Liquid region
  Real z = 0.0001;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verify fluid density and viscosity
  _fp->liquidProperties(p, T, xnacl, fsp);

  Real liquid_density = fsp[0].density;
  Real liquid_viscosity = fsp[0].viscosity;

  Real co2_partial_density, dco2_partial_density_dT;
  _fp->partialDensityCO2(T, co2_partial_density, dco2_partial_density_dT);
  Real brine_density = _brine_fp->rho(p, T, xnacl);

  Real density = 1.0 / (z / co2_partial_density + (1.0 - z) / brine_density);

  Real water_density = _water_fp->rho(p, T);
  Real viscosity = _brine_fp->mu_from_rho_T(water_density, T, xnacl);

  ABS_TEST("liquid density", liquid_density, density, 1.0e-12);
  ABS_TEST("liquid viscosity", liquid_viscosity, viscosity, 1.0e-12);

  // Verify fluid density and viscosity derivatives
  Real ddensity_dp = fsp[0].ddensity_dp;
  Real ddensity_dT = fsp[0].ddensity_dT;
  Real ddensity_dz = fsp[0].ddensity_dz;
  Real dviscosity_dp = fsp[0].dviscosity_dp;
  Real dviscosity_dT = fsp[0].dviscosity_dT;
  Real dviscosity_dz = fsp[0].dviscosity_dz;

  const Real dp = 1.0;
  _fp->liquidProperties(p + dp, T, xnacl, fsp);
  Real rho1 = fsp[0].density;
  Real mu1 = fsp[0].viscosity;

  _fp->liquidProperties(p - dp, T, xnacl, fsp);
  Real rho2 = fsp[0].density;
  Real mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);

  const Real dT = 1.0e-4;
  _fp->liquidProperties(p, T + dT, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->liquidProperties(p, T - dT, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);

  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-6);

  // Two-phase region
  z = 0.045;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Verify fluid density and viscosity derivatives
  _fp->liquidProperties(p, T, xnacl, fsp);

  ddensity_dp = fsp[0].ddensity_dp;
  ddensity_dT = fsp[0].ddensity_dT;
  ddensity_dz = fsp[0].ddensity_dz;
  dviscosity_dp = fsp[0].dviscosity_dp;
  dviscosity_dT = fsp[0].dviscosity_dT;
  dviscosity_dz = fsp[0].dviscosity_dz;

  _fp->massFractions(p + dp, T, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p + dp, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->massFractions(p - dp, T, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p - dp, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);

  _fp->massFractions(p, T + dT, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p, T + dT, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->massFractions(p, T - dT, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p, T - dT, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);

  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  ABS_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-6);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowBrineCO2Test, saturationTwoPhase)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // In the two-phase region, the mass fractions are the equilibrium values, so
  // a temporary value of z can be used (as long as it corresponds to the two-phase
  // region)
  Real z = 0.45;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate z that gives a saturation of 0.25
  Real gas_saturation = 0.25;
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - gas_saturation);
  // Calculate gas density and liquid density
  _fp->gasProperties(p, T, fsp);
  _fp->liquidProperties(liquid_pressure, T, xnacl, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  z = (gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
       (1.0 - gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
      (gas_saturation * fsp[1].density + (1.0 - gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  _fp->saturationTwoPhase(p, T, xnacl, z, fsp);

  ABS_TEST("gas saturation", fsp[1].saturation, gas_saturation, 1.0e-8);

  // Test the derivatives
  const Real dp = 1.0e-1;
  gas_saturation = fsp[1].saturation;
  Real dgas_saturation_dp = fsp[1].dsaturation_dp;
  Real dgas_saturation_dT = fsp[1].dsaturation_dT;
  Real dgas_saturation_dz = fsp[1].dsaturation_dz;

  _fp->massFractions(p + dp, T, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  _fp->saturationTwoPhase(p + dp, T, xnacl, z, fsp);
  Real gsat1 = fsp[1].saturation;

  _fp->massFractions(p - dp, T, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  _fp->saturationTwoPhase(p - dp, T, xnacl, z, fsp);
  Real gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dp", dgas_saturation_dp, (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt T
  const Real dT = 1.0e-4;
  _fp->massFractions(p, T + dT, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  _fp->saturationTwoPhase(p, T + dT, xnacl, z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T - dT, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  _fp->saturationTwoPhase(p, T - dT, xnacl, z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dT", dgas_saturation_dT, (gsat1 - gsat2) / (2.0 * dT), 1.0e-6);

  // Derivative wrt z
  const Real dz = 1.0e-8;

  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, xnacl, z + dz, fsp);
  gsat1 = fsp[1].saturation;

  _fp->saturationTwoPhase(p, T, xnacl, z - dz, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dz", dgas_saturation_dz, (gsat1 - gsat2) / (2.0 * dz), 1.0e-6);
}

/*
 * Verify calculation of total mass fraction given a gas saturation
 */
TEST_F(PorousFlowBrineCO2Test, totalMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;
  const Real s = 0.2;

  Real z = _fp->totalMassFraction(p, T, xnacl, s);

  // Test that the saturation calculated in this fluid state using z is equal to s
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - s);
  _fp->liquidProperties(liquid_pressure, T, xnacl, fsp);
  _fp->saturationTwoPhase(p, T, xnacl, z, fsp);
  ABS_TEST("gas saturation", fsp[1].saturation, s, 1.0e-8);
}

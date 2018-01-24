//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterNCGTest.h"

/**
 * Verify that the correct name is supplied
 */
TEST_F(PorousFlowWaterNCGTest, name) { EXPECT_EQ("water-ncg", _fp->fluidStateName()); }

/*
 * Verify calculation of equilibrium mass fraction and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, equilibriumMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;
  Real Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT;
  Real Xncg1, dXncg1_dp, dXncg1_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT;
  Real Xncg2, dXncg2_dp, dXncg2_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT;
  _fp->equilibriumMassFractions(p, T, Xncg, dXncg_dp, dXncg_dT, Yh2o, dYh2o_dp, dYh2o_dT);
  _fp->equilibriumMassFractions(
      p - dp, T, Xncg1, dXncg1_dp, dXncg1_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT);
  _fp->equilibriumMassFractions(
      p + dp, T, Xncg2, dXncg2_dp, dXncg2_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT);

  Real dXncg_dp_fd = (Xncg2 - Xncg1) / (2.0 * dp);
  Real dYh2o_dp_fd = (Yh2o2 - Yh2o1) / (2.0 * dp);

  REL_TEST("dXncg_dp", dXncg_dp, dXncg_dp_fd, 1.0e-6);
  REL_TEST("dYh2o_dp", dYh2o_dp, dYh2o_dp_fd, 1.0e-6);

  _fp->equilibriumMassFractions(
      p, T - dT, Xncg1, dXncg1_dp, dXncg1_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT);
  _fp->equilibriumMassFractions(
      p, T + dT, Xncg2, dXncg2_dp, dXncg2_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT);

  Real dXncg_dT_fd = (Xncg2 - Xncg1) / (2.0 * dT);
  Real dYh2o_dT_fd = (Yh2o2 - Yh2o1) / (2.0 * dT);

  REL_TEST("dXncg_dT", dXncg_dT, dXncg_dT_fd, 1.0e-6);
  REL_TEST("dYh2o_dT", dYh2o_dT, dYh2o_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of actual mass fraction and derivatives depending on value of
 * total mass fraction
 */
TEST_F(PorousFlowWaterNCGTest, MassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Liquid region
  Real z = 0.0001;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  Real Xncg = fsp[0].mass_fraction[1];
  Real Yncg = fsp[1].mass_fraction[1];
  Real Xh2o = fsp[0].mass_fraction[0];
  Real Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xncg", Xncg, z, 1.0e-8);
  ABS_TEST("Yncg", Yncg, 0.0, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - z, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 0.0, 1.0e-8);

  // Verify derivatives
  Real dXncg_dp = fsp[0].dmass_fraction_dp[1];
  Real dXncg_dT = fsp[0].dmass_fraction_dT[1];
  Real dXncg_dz = fsp[0].dmass_fraction_dz[1];
  Real dYncg_dp = fsp[1].dmass_fraction_dp[1];
  Real dYncg_dT = fsp[1].dmass_fraction_dT[1];
  Real dYncg_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXncg_dp", dXncg_dp, 0.0, 1.0e-8);
  ABS_TEST("dXncg_dT", dXncg_dT, 0.0, 1.0e-8);
  ABS_TEST("dXncg_dz", dXncg_dz, 1.0, 1.0e-8);
  ABS_TEST("dYncg_dp", dYncg_dp, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dT", dYncg_dT, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dz", dYncg_dz, 0.0, 1.0e-8);

  // Gas region
  z = 0.995;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xncg = fsp[0].mass_fraction[1];
  Yncg = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xncg", Xncg, 0.0, 1.0e-8);
  ABS_TEST("Yncg", Yncg, z, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 0.0, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 1.0 - z, 1.0e-8);

  // Verify derivatives
  dXncg_dp = fsp[0].dmass_fraction_dp[1];
  dXncg_dT = fsp[0].dmass_fraction_dT[1];
  dXncg_dz = fsp[0].dmass_fraction_dz[1];
  dYncg_dp = fsp[1].dmass_fraction_dp[1];
  dYncg_dT = fsp[1].dmass_fraction_dT[1];
  dYncg_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXncg_dp", dXncg_dp, 0.0, 1.0e-8);
  ABS_TEST("dXncg_dT", dXncg_dT, 0.0, 1.0e-8);
  ABS_TEST("dXncg_dz", dXncg_dz, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dp", dYncg_dp, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dT", dYncg_dT, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dz", dYncg_dz, 1.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  //  be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  z = 0.45;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  Real Xncg_eq, dXncg_dp_eq, dXncg_dT_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq;
  _fp->equilibriumMassFractions(
      p, T, Xncg_eq, dXncg_dp_eq, dXncg_dT_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq);

  // Verfify mass fraction values
  Xncg = fsp[0].mass_fraction[1];
  Yncg = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xncg", Xncg, Xncg_eq, 1.0e-8);
  ABS_TEST("Yncg", Yncg, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - Xncg_eq, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, Yh2o_eq, 1.0e-8);

  // Verify derivatives wrt p and T
  dXncg_dp = fsp[0].dmass_fraction_dp[1];
  dXncg_dT = fsp[0].dmass_fraction_dT[1];
  dXncg_dz = fsp[0].dmass_fraction_dz[1];
  dYncg_dp = fsp[1].dmass_fraction_dp[1];
  dYncg_dT = fsp[1].dmass_fraction_dT[1];
  dYncg_dz = fsp[1].dmass_fraction_dz[1];
  ABS_TEST("dXncg_dp", dXncg_dp, dXncg_dp_eq, 1.0e-8);
  ABS_TEST("dXncg_dT", dXncg_dT, dXncg_dT_eq, 1.0e-8);
  ABS_TEST("dXncg_dz", dXncg_dz, 0.0, 1.0e-8);
  ABS_TEST("dYncg_dp", dYncg_dp, -dYh2o_dp_eq, 1.0e-8);
  ABS_TEST("dYncg_dT", dYncg_dT, -dYh2o_dT_eq, 1.0e-8);
  ABS_TEST("dYncg_dz", dYncg_dz, 0.0, 1.0e-8);

  // Use finite differences to verify derivative wrt z is unaffected by z
  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, z + dz, phase_state, fsp);
  Real Xncg1 = fsp[0].mass_fraction[1];
  Real Yncg1 = fsp[1].mass_fraction[1];
  _fp->massFractions(p, T, z - dz, phase_state, fsp);
  Real Xncg2 = fsp[0].mass_fraction[1];
  Real Yncg2 = fsp[1].mass_fraction[1];

  ABS_TEST("dXncg_dz", dXncg_dz, (Xncg1 - Xncg2) / (2.0 * dz), 1.0e-8);
  ABS_TEST("dXYncg_dz", dYncg_dz, (Yncg1 - Yncg2) / (2.0 * dz), 1.0e-8);
}

/*
 * Verify calculation of gas density and viscosity, and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, gasProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Gas region
  Real z = 0.995;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density and viscosity
  _fp->gasProperties(p, T, fsp);
  Real gas_density = fsp[1].density;
  Real gas_viscosity = fsp[1].viscosity;
  Real density = _ncg_fp->rho(z * p, T) + _water_fp->rho(_water_fp->vaporPressure(T), T);
  Real viscosity =
      z * _ncg_fp->mu(z * p, T) + (1.0 - z) * _water_fp->mu(_water_fp->vaporPressure(T), T);

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
  _fp->massFractions(p, T, z + dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;

  _fp->massFractions(p, T, z - dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  REL_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  REL_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-6);

  // Check derivatives in the two phase region as well. Note that the mass fractions
  // vary with pressure and temperature in this region
  z = 0.45;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  ddensity_dp = fsp[1].ddensity_dp;
  ddensity_dT = fsp[1].ddensity_dT;
  ddensity_dz = fsp[1].ddensity_dz;
  dviscosity_dp = fsp[1].dviscosity_dp;
  dviscosity_dT = fsp[1].dviscosity_dT;
  dviscosity_dz = fsp[1].dviscosity_dz;

  _fp->massFractions(p + dp, T, z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;

  _fp->massFractions(p - dp, T, z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);

  _fp->massFractions(p, T + dT, z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;

  _fp->massFractions(p, T - dT, z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);

  _fp->massFractions(p, T, z + dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;

  _fp->massFractions(p, T, z - dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;

  ABS_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("dviscosity_dz", dviscosity_dT, (mu1 - mu2) / (2.0 * dz), 1.0e-6);
}

/*
 * Verify calculation of liquid density and viscosity, and derivatives. Note that as
 * these properties don't depend on mass fraction, only the liquid region needs to be
 * tested (the calculations are identical in the two phase region)
 */
TEST_F(PorousFlowWaterNCGTest, liquidProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Verify fluid density and viscosity
  _fp->liquidProperties(p, T, fsp);
  Real liquid_density = fsp[0].density;
  Real liquid_viscosity = fsp[0].viscosity;
  Real density = _water_fp->rho(p, T);
  Real viscosity = _water_fp->mu(p, T);

  ABS_TEST("liquid density", liquid_density, density, 1.0e-12);
  ABS_TEST("liquid viscosity", liquid_viscosity, viscosity, 1.0e-12);

  // Verify derivatives
  Real ddensity_dp = fsp[0].ddensity_dp;
  Real ddensity_dT = fsp[0].ddensity_dT;
  Real ddensity_dz = fsp[0].ddensity_dz;
  Real dviscosity_dp = fsp[0].dviscosity_dp;
  Real dviscosity_dT = fsp[0].dviscosity_dT;
  Real dviscosity_dz = fsp[0].dviscosity_dz;

  const Real dp = 1.0;
  _fp->liquidProperties(p + dp, T, fsp);
  Real rho1 = fsp[0].density;
  Real mu1 = fsp[0].viscosity;

  _fp->liquidProperties(p - dp, T, fsp);
  Real rho2 = fsp[0].density;
  Real mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);

  const Real dT = 1.0e-4;
  _fp->liquidProperties(p, T + dT, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;

  _fp->liquidProperties(p, T - dT, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);

  ABS_TEST("ddensity_dz", ddensity_dz, 0.0, 1.0e-12);
  ABS_TEST("dviscosity_dz", dviscosity_dz, 0.0, 1.0e-12);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowWaterNCGTest, twoPhase)
{
  const Real p = 1.0e6;
  const Real T = 350.0;

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // In the two-phase region, the mass fractions are the equilibrium values, so
  // a temporary value of z can be used (as long as it corresponds to the two-phase
  // region)
  Real z = 0.45;
  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate z that gives a saturation of 0.25
  Real gas_saturation = 0.25;
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - gas_saturation);
  // Calculate gas density and liquid density
  _fp->gasProperties(p, T, fsp);
  _fp->liquidProperties(liquid_pressure, T, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  z = (gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
       (1.0 - gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
      (gas_saturation * fsp[1].density + (1.0 - gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  _fp->saturationTwoPhase(p, T, z, fsp);

  ABS_TEST("gas saturation", fsp[1].saturation, gas_saturation, 1.0e-8);

  // Test the derivatives
  const Real dp = 1.0e-1;
  gas_saturation = fsp[1].saturation;
  Real dgas_saturation_dp = fsp[1].dsaturation_dp;
  Real dgas_saturation_dT = fsp[1].dsaturation_dT;
  Real dgas_saturation_dz = fsp[1].dsaturation_dz;

  _fp->massFractions(p + dp, T, z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  _fp->saturationTwoPhase(p + dp, T, z, fsp);
  Real gsat1 = fsp[1].saturation;

  _fp->massFractions(p - dp, T, z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  _fp->saturationTwoPhase(p - dp, T, z, fsp);
  Real gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dp", dgas_saturation_dp, (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt T
  const Real dT = 1.0e-4;
  _fp->massFractions(p, T + dT, z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  _fp->saturationTwoPhase(p, T + dT, z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T - dT, z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  _fp->saturationTwoPhase(p, T - dT, z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dT", dgas_saturation_dT, (gsat1 - gsat2) / (2.0 * dT), 1.0e-6);

  // Derivative wrt z
  const Real dz = 1.0e-8;

  _fp->massFractions(p, T, z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, z + dz, fsp);
  gsat1 = fsp[1].saturation;

  _fp->saturationTwoPhase(p, T, z - dz, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dz", dgas_saturation_dz, (gsat1 - gsat2) / (2.0 * dz), 1.0e-6);
}

/*
 * Verify calculation of total mass fraction given a gas saturation
 */
TEST_F(PorousFlowWaterNCGTest, totalMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real s = 0.2;

  Real z = _fp->totalMassFraction(p, T, s);

  // Test that the saturation calculated in this fluid state using z is equal to s
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  _fp->massFractions(p, T, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - s);
  _fp->liquidProperties(liquid_pressure, T, fsp);
  _fp->saturationTwoPhase(p, T, z, fsp);
  ABS_TEST("gas saturation", fsp[1].saturation, s, 1.0e-8);
}

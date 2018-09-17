//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterNCGTest.h"
#include "FluidPropertiesTestUtils.h"

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

  REL_TEST(dXncg_dp, dXncg_dp_fd, 1.0e-6);
  REL_TEST(dYh2o_dp, dYh2o_dp_fd, 1.0e-6);

  _fp->equilibriumMassFractions(
      p, T - dT, Xncg1, dXncg1_dp, dXncg1_dT, Yh2o1, dYh2o1_dp, dYh2o1_dT);
  _fp->equilibriumMassFractions(
      p, T + dT, Xncg2, dXncg2_dp, dXncg2_dT, Yh2o2, dYh2o2_dp, dYh2o2_dT);

  Real dXncg_dT_fd = (Xncg2 - Xncg1) / (2.0 * dT);
  Real dYh2o_dT_fd = (Yh2o2 - Yh2o1) / (2.0 * dT);

  REL_TEST(dXncg_dT, dXncg_dT_fd, 1.0e-6);
  REL_TEST(dYh2o_dT, dYh2o_dT_fd, 1.0e-6);
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
  Real Z = 0.0001;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  Real Xncg = fsp[0].mass_fraction[1];
  Real Yncg = fsp[1].mass_fraction[1];
  Real Xh2o = fsp[0].mass_fraction[0];
  Real Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xncg, Z, 1.0e-8);
  ABS_TEST(Yncg, 0.0, 1.0e-8);
  ABS_TEST(Xh2o, 1.0 - Z, 1.0e-8);
  ABS_TEST(Yh2o, 0.0, 1.0e-8);

  // Verify derivatives
  Real dXncg_dp = fsp[0].dmass_fraction_dp[1];
  Real dXncg_dT = fsp[0].dmass_fraction_dT[1];
  Real dXncg_dZ = fsp[0].dmass_fraction_dZ[1];
  Real dYncg_dp = fsp[1].dmass_fraction_dp[1];
  Real dYncg_dT = fsp[1].dmass_fraction_dT[1];
  Real dYncg_dZ = fsp[1].dmass_fraction_dZ[1];
  ABS_TEST(dXncg_dp, 0.0, 1.0e-8);
  ABS_TEST(dXncg_dT, 0.0, 1.0e-8);
  ABS_TEST(dXncg_dZ, 1.0, 1.0e-8);
  ABS_TEST(dYncg_dp, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dT, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dZ, 0.0, 1.0e-8);

  // Gas region
  Z = 0.995;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xncg = fsp[0].mass_fraction[1];
  Yncg = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xncg, 0.0, 1.0e-8);
  ABS_TEST(Yncg, Z, 1.0e-8);
  ABS_TEST(Xh2o, 0.0, 1.0e-8);
  ABS_TEST(Yh2o, 1.0 - Z, 1.0e-8);

  // Verify derivatives
  dXncg_dp = fsp[0].dmass_fraction_dp[1];
  dXncg_dT = fsp[0].dmass_fraction_dT[1];
  dXncg_dZ = fsp[0].dmass_fraction_dZ[1];
  dYncg_dp = fsp[1].dmass_fraction_dp[1];
  dYncg_dT = fsp[1].dmass_fraction_dT[1];
  dYncg_dZ = fsp[1].dmass_fraction_dZ[1];
  ABS_TEST(dXncg_dp, 0.0, 1.0e-8);
  ABS_TEST(dXncg_dT, 0.0, 1.0e-8);
  ABS_TEST(dXncg_dZ, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dp, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dT, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dZ, 1.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  //  be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  Z = 0.45;
  _fp->massFractions(p, T, Z, phase_state, fsp);
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
  ABS_TEST(Xncg, Xncg_eq, 1.0e-8);
  ABS_TEST(Yncg, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST(Xh2o, 1.0 - Xncg_eq, 1.0e-8);
  ABS_TEST(Yh2o, Yh2o_eq, 1.0e-8);

  // Verify derivatives wrt p and T
  dXncg_dp = fsp[0].dmass_fraction_dp[1];
  dXncg_dT = fsp[0].dmass_fraction_dT[1];
  dXncg_dZ = fsp[0].dmass_fraction_dZ[1];
  dYncg_dp = fsp[1].dmass_fraction_dp[1];
  dYncg_dT = fsp[1].dmass_fraction_dT[1];
  dYncg_dZ = fsp[1].dmass_fraction_dZ[1];
  ABS_TEST(dXncg_dp, dXncg_dp_eq, 1.0e-8);
  ABS_TEST(dXncg_dT, dXncg_dT_eq, 1.0e-8);
  ABS_TEST(dXncg_dZ, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dp, -dYh2o_dp_eq, 1.0e-8);
  ABS_TEST(dYncg_dT, -dYh2o_dT_eq, 1.0e-8);
  ABS_TEST(dYncg_dZ, 0.0, 1.0e-8);

  // Use finite differences to verify derivative wrt Z is unaffected by Z
  const Real dZ = 1.0e-8;
  _fp->massFractions(p, T, Z + dZ, phase_state, fsp);
  Real Xncg1 = fsp[0].mass_fraction[1];
  Real Yncg1 = fsp[1].mass_fraction[1];
  _fp->massFractions(p, T, Z - dZ, phase_state, fsp);
  Real Xncg2 = fsp[0].mass_fraction[1];
  Real Yncg2 = fsp[1].mass_fraction[1];

  ABS_TEST(dXncg_dZ, (Xncg1 - Xncg2) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(dYncg_dZ, (Yncg1 - Yncg2) / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of gas density, viscosity, enthalpy and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, gasProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Gas region
  Real Z = 0.995;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density, viscosity and enthalpy
  _fp->gasProperties(p, T, fsp);
  Real gas_density = fsp[1].density;
  Real gas_viscosity = fsp[1].viscosity;
  Real gas_enthalpy = fsp[1].enthalpy;

  Real density =
      _ncg_fp->rho_from_p_T(Z * p, T) + _water_fp->rho_from_p_T(_water_fp->vaporPressure(T), T);
  Real viscosity = Z * _ncg_fp->mu_from_p_T(Z * p, T) +
                   (1.0 - Z) * _water_fp->mu_from_p_T(_water_fp->vaporPressure(T), T);
  Real enthalpy = Z * _ncg_fp->h_from_p_T(Z * p, T) +
                  (1.0 - Z) * _water_fp->h_from_p_T(_water_fp->vaporPressure(T), T);

  ABS_TEST(gas_density, density, 1.0e-8);
  ABS_TEST(gas_viscosity, viscosity, 1.0e-8);
  ABS_TEST(gas_enthalpy, enthalpy, 1.0e-8);

  // Verify derivatives
  Real ddensity_dp = fsp[1].ddensity_dp;
  Real ddensity_dT = fsp[1].ddensity_dT;
  Real ddensity_dZ = fsp[1].ddensity_dZ;
  Real dviscosity_dp = fsp[1].dviscosity_dp;
  Real dviscosity_dT = fsp[1].dviscosity_dT;
  Real dviscosity_dZ = fsp[1].dviscosity_dZ;
  Real denthalpy_dp = fsp[1].denthalpy_dp;
  Real denthalpy_dT = fsp[1].denthalpy_dT;
  Real denthalpy_dZ = fsp[1].denthalpy_dZ;

  const Real dp = 1.0e-1;
  _fp->gasProperties(p + dp, T, fsp);
  Real rho1 = fsp[1].density;
  Real mu1 = fsp[1].viscosity;
  Real h1 = fsp[1].enthalpy;

  _fp->gasProperties(p - dp, T, fsp);
  Real rho2 = fsp[1].density;
  Real mu2 = fsp[1].viscosity;
  Real h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-6);

  const Real dT = 1.0e-3;
  _fp->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Note: mass fraction changes with Z
  const Real dZ = 1.0e-8;
  _fp->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dZ, (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  REL_TEST(dviscosity_dZ, (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  REL_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);

  // Check derivatives in the two phase region as well. Note that the mass fractions
  // vary with pressure and temperature in this region
  Z = 0.45;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  ddensity_dp = fsp[1].ddensity_dp;
  ddensity_dT = fsp[1].ddensity_dT;
  ddensity_dZ = fsp[1].ddensity_dZ;
  dviscosity_dp = fsp[1].dviscosity_dp;
  dviscosity_dT = fsp[1].dviscosity_dT;
  dviscosity_dZ = fsp[1].dviscosity_dZ;
  denthalpy_dp = fsp[1].denthalpy_dp;
  denthalpy_dT = fsp[1].denthalpy_dT;
  denthalpy_dZ = fsp[1].denthalpy_dZ;

  _fp->massFractions(p + dp, T, Z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p - dp, T, Z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-6);

  _fp->massFractions(p, T + dT, Z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p, T - dT, Z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  _fp->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  ABS_TEST(ddensity_dZ, (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of liquid density, viscosity, enthalpy and derivatives. Note that as
 * density and viscosity don't depend on mass fraction, only the liquid region needs to be
 * tested (the calculations are identical in the two phase region). The enthalpy does depend
 * on mass fraction, so should be tested in the two phase region as well as the liquid region
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
  Real density = _water_fp->rho_from_p_T(p, T);
  Real viscosity = _water_fp->mu_from_p_T(p, T);

  ABS_TEST(liquid_density, density, 1.0e-12);
  ABS_TEST(liquid_viscosity, viscosity, 1.0e-12);

  // Verify derivatives
  Real ddensity_dp = fsp[0].ddensity_dp;
  Real ddensity_dT = fsp[0].ddensity_dT;
  Real ddensity_dZ = fsp[0].ddensity_dZ;
  Real dviscosity_dp = fsp[0].dviscosity_dp;
  Real dviscosity_dT = fsp[0].dviscosity_dT;
  Real dviscosity_dZ = fsp[0].dviscosity_dZ;
  Real denthalpy_dp = fsp[0].denthalpy_dp;
  Real denthalpy_dT = fsp[0].denthalpy_dT;
  Real denthalpy_dZ = fsp[0].denthalpy_dZ;

  const Real dp = 1.0;
  _fp->liquidProperties(p + dp, T, fsp);
  Real rho1 = fsp[0].density;
  Real mu1 = fsp[0].viscosity;
  Real h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p - dp, T, fsp);
  Real rho2 = fsp[0].density;
  Real mu2 = fsp[0].viscosity;
  Real h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-5);

  const Real dT = 1.0e-4;
  _fp->liquidProperties(p, T + dT, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p, T - dT, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-5);

  Real Z = 0.0001;
  const Real dZ = 1.0e-8;
  FluidStatePhaseEnum phase_state;

  _fp->massFractions(p, T, Z, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  denthalpy_dp = fsp[0].denthalpy_dp;
  denthalpy_dT = fsp[0].denthalpy_dT;
  denthalpy_dZ = fsp[0].denthalpy_dZ;

  _fp->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);

  // Density and viscosity don't depend on Z, so derivatives should be 0
  ABS_TEST(ddensity_dZ, 0.0, 1.0e-12);
  ABS_TEST(dviscosity_dZ, 0.0, 1.0e-12);

  // Check enthalpy calculations in the two phase region as well. Note that the mass fractions
  // vary with pressure and temperature in this region
  Z = 0.45;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  denthalpy_dp = fsp[0].denthalpy_dp;
  denthalpy_dT = fsp[0].denthalpy_dT;
  denthalpy_dZ = fsp[0].denthalpy_dZ;

  _fp->massFractions(p + dp, T, Z, phase_state, fsp);
  _fp->liquidProperties(p + dp, T, fsp);
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p - dp, T, Z, phase_state, fsp);
  _fp->liquidProperties(p - dp, T, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-5);

  _fp->massFractions(p, T + dT, Z, phase_state, fsp);
  _fp->liquidProperties(p, T + dT, fsp);
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T - dT, Z, phase_state, fsp);
  _fp->liquidProperties(p, T - dT, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-5);

  _fp->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, fsp);
  h2 = fsp[0].enthalpy;

  ABS_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-5);
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
  // a temporary value of Z can be used (as long as it corresponds to the two-phase
  // region)
  Real Z = 0.45;
  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate Z that gives a saturation of 0.25
  Real gas_saturation = 0.25;
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - gas_saturation);
  // Calculate gas density and liquid density
  _fp->gasProperties(p, T, fsp);
  _fp->liquidProperties(liquid_pressure, T, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  Z = (gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
       (1.0 - gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
      (gas_saturation * fsp[1].density + (1.0 - gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  _fp->saturationTwoPhase(p, T, Z, fsp);

  ABS_TEST(fsp[1].saturation, gas_saturation, 1.0e-8);

  // Test the derivatives
  const Real dp = 1.0e-1;
  gas_saturation = fsp[1].saturation;
  Real dgas_saturation_dp = fsp[1].dsaturation_dp;
  Real dgas_saturation_dT = fsp[1].dsaturation_dT;
  Real dgas_saturation_dZ = fsp[1].dsaturation_dZ;

  _fp->massFractions(p + dp, T, Z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  _fp->saturationTwoPhase(p + dp, T, Z, fsp);
  Real gsat1 = fsp[1].saturation;

  _fp->massFractions(p - dp, T, Z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  _fp->saturationTwoPhase(p - dp, T, Z, fsp);
  Real gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dp, (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt T
  const Real dT = 1.0e-4;
  _fp->massFractions(p, T + dT, Z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  _fp->saturationTwoPhase(p, T + dT, Z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T - dT, Z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  _fp->saturationTwoPhase(p, T - dT, Z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dT, (gsat1 - gsat2) / (2.0 * dT), 1.0e-6);

  // Derivative wrt Z
  const Real dZ = 1.0e-8;

  _fp->massFractions(p, T, Z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, Z + dZ, fsp);
  gsat1 = fsp[1].saturation;

  _fp->saturationTwoPhase(p, T, Z - dZ, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dZ, (gsat1 - gsat2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of total mass fraction given a gas saturation
 */
TEST_F(PorousFlowWaterNCGTest, totalMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real s = 0.2;

  Real Z = _fp->totalMassFraction(p, T, s);

  // Test that the saturation calculated in this fluid state using Z is equal to s
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  _fp->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - s);
  _fp->liquidProperties(liquid_pressure, T, fsp);
  _fp->saturationTwoPhase(p, T, Z, fsp);
  ABS_TEST(fsp[1].saturation, s, 1.0e-8);
}

/*
 * Verify calculation of enthalpy of dissolution. Note: the values calculated compare
 * well by eye to the values presented in Figure 4 of Battistelli et al, "A fluid property
 * module for the TOUGH2 simulator for saline brines with non-condensible gas"
 */
TEST_F(PorousFlowWaterNCGTest, enthalpyOfDissolution)
{
  // T = 50C
  Real T = 323.15;

  // Enthalpy of dissolution of NCG in water
  Real hdis, dhdis_dT;
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  REL_TEST(hdis, -3.45731e5, 1.0e-3);

  // T = 350C
  T = 623.15;

  // Enthalpy of dissolution of NCG in water
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  REL_TEST(hdis, 1.23423e+06, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis2, dhdis2_dT;
  _fp->enthalpyOfDissolution(T + dT, hdis, dhdis_dT);
  _fp->enthalpyOfDissolution(T - dT, hdis2, dhdis2_dT);

  REL_TEST(dhdis_dT, (hdis - hdis2) / (2.0 * dT), 1.0e-5);
}

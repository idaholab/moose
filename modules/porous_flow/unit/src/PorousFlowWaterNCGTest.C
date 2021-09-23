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
TEST_F(PorousFlowWaterNCGTest, name) { EXPECT_EQ("water-ncg", _fs->fluidStateName()); }

/*
 * Verify calculation of equilibrium mass fraction and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, equilibriumMassFraction)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;

  DualReal Xncg, Yh2o, Xncg1, Yh2o1, Xncg2, Yh2o2;
  _fs->equilibriumMassFractions(p, T, Xncg, Yh2o);
  _fs->equilibriumMassFractions(p - dp, T, Xncg1, Yh2o1);
  _fs->equilibriumMassFractions(p + dp, T, Xncg2, Yh2o2);

  Real dXncg_dp_fd = (Xncg2.value() - Xncg1.value()) / (2.0 * dp);
  Real dYh2o_dp_fd = (Yh2o2.value() - Yh2o1.value()) / (2.0 * dp);

  REL_TEST(Xncg.derivatives()[_pidx], dXncg_dp_fd, 1.0e-8);
  REL_TEST(Yh2o.derivatives()[_pidx], dYh2o_dp_fd, 1.0e-7);

  _fs->equilibriumMassFractions(p, T - dT, Xncg1, Yh2o1);
  _fs->equilibriumMassFractions(p, T + dT, Xncg2, Yh2o2);

  Real dXncg_dT_fd = (Xncg2.value() - Xncg1.value()) / (2.0 * dT);
  Real dYh2o_dT_fd = (Yh2o2.value() - Yh2o1.value()) / (2.0 * dT);

  REL_TEST(Xncg.derivatives()[_Tidx], dXncg_dT_fd, 1.0e-7);
  REL_TEST(Yh2o.derivatives()[_Tidx], dYh2o_dT_fd, 1.0e-7);
}

/*
 * Verify calculation of actual mass fraction and derivatives depending on value of
 * total mass fraction
 */
TEST_F(PorousFlowWaterNCGTest, MassFraction)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Liquid region
  DualReal Z = 0.0001;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  DualReal Xncg = fsp[0].mass_fraction[1];
  DualReal Yncg = fsp[1].mass_fraction[1];
  DualReal Xh2o = fsp[0].mass_fraction[0];
  DualReal Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xncg.value(), Z.value(), 1.0e-12);
  ABS_TEST(Yncg.value(), 0.0, 1.0e-12);
  ABS_TEST(Xh2o.value(), 1.0 - Z.value(), 1.0e-12);
  ABS_TEST(Yh2o.value(), 0.0, 1.0e-12);

  // Verify derivatives
  Real dXncg_dp = Xncg.derivatives()[_pidx];
  Real dXncg_dT = Xncg.derivatives()[_Tidx];
  Real dXncg_dZ = Xncg.derivatives()[_Zidx];
  Real dYncg_dp = Yncg.derivatives()[_pidx];
  Real dYncg_dT = Yncg.derivatives()[_Tidx];
  Real dYncg_dZ = Yncg.derivatives()[_Zidx];
  ABS_TEST(dXncg_dp, 0.0, 1.0e-12);
  ABS_TEST(dXncg_dT, 0.0, 1.0e-12);
  ABS_TEST(dXncg_dZ, 1.0, 1.0e-12);
  ABS_TEST(dYncg_dp, 0.0, 1.0e-12);
  ABS_TEST(dYncg_dT, 0.0, 1.0e-12);
  ABS_TEST(dYncg_dZ, 0.0, 1.0e-12);

  // Gas region
  Z = 0.995;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xncg = fsp[0].mass_fraction[1];
  Yncg = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xncg.value(), 0.0, 1.0e-12);
  ABS_TEST(Yncg.value(), Z.value(), 1.0e-12);
  ABS_TEST(Xh2o.value(), 0.0, 1.0e-12);
  ABS_TEST(Yh2o.value(), 1.0 - Z.value(), 1.0e-12);

  // Verify derivatives
  dXncg_dp = Xncg.derivatives()[_pidx];
  dXncg_dT = Xncg.derivatives()[_Tidx];
  dXncg_dZ = Xncg.derivatives()[_Zidx];
  dYncg_dp = Yncg.derivatives()[_pidx];
  dYncg_dT = Yncg.derivatives()[_Tidx];
  dYncg_dZ = Yncg.derivatives()[_Zidx];
  ABS_TEST(dXncg_dp, 0.0, 1.0e-12);
  ABS_TEST(dXncg_dT, 0.0, 1.0e-12);
  ABS_TEST(dXncg_dZ, 0.0, 1.0e-12);
  ABS_TEST(dYncg_dp, 0.0, 1.0e-12);
  ABS_TEST(dYncg_dT, 0.0, 1.0e-12);
  ABS_TEST(dYncg_dZ, 1.0, 1.0e-12);

  // Two phase region. In this region, the mass fractions and derivatives can
  //  be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  DualReal Xncg_eq, Yh2o_eq;
  _fs->equilibriumMassFractions(p, T, Xncg_eq, Yh2o_eq);

  // Verfify mass fraction values
  Xncg = fsp[0].mass_fraction[1];
  Yncg = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xncg, Xncg_eq, 1.0e-12);
  ABS_TEST(Yncg, 1.0 - Yh2o_eq, 1.0e-12);
  ABS_TEST(Xh2o, 1.0 - Xncg_eq, 1.0e-12);
  ABS_TEST(Yh2o, Yh2o_eq, 1.0e-12);

  // Verify derivatives wrt p and T
  dXncg_dp = Xncg.derivatives()[_pidx];
  dXncg_dT = Xncg.derivatives()[_Tidx];
  dXncg_dZ = Xncg.derivatives()[_Zidx];
  dYncg_dp = Yncg.derivatives()[_pidx];
  dYncg_dT = Yncg.derivatives()[_Tidx];
  dYncg_dZ = Yncg.derivatives()[_Zidx];
  ABS_TEST(dXncg_dp, Xncg_eq.derivatives()[_pidx], 1.0e-8);
  ABS_TEST(dXncg_dT, Xncg_eq.derivatives()[_Tidx], 1.0e-8);
  ABS_TEST(dXncg_dZ, 0.0, 1.0e-8);
  ABS_TEST(dYncg_dp, -Yh2o_eq.derivatives()[_pidx], 1.0e-8);
  ABS_TEST(dYncg_dT, -Yh2o_eq.derivatives()[_Tidx], 1.0e-8);
  ABS_TEST(dYncg_dZ, 0.0, 1.0e-8);

  // Use finite differences to verify derivative wrt Z is unaffected by Z
  const Real dZ = 1.0e-8;
  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  DualReal Xncg1 = fsp[0].mass_fraction[1];
  DualReal Yncg1 = fsp[1].mass_fraction[1];
  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  DualReal Xncg2 = fsp[0].mass_fraction[1];
  DualReal Yncg2 = fsp[1].mass_fraction[1];

  ABS_TEST(dXncg_dZ, (Xncg1 - Xncg2).value() / (2.0 * dZ), 1.0e-8);
  ABS_TEST(dYncg_dZ, (Yncg1 - Yncg2).value() / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of gas density and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, gasDensity)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Gas region
  DualReal Z = 0.995;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  const DualReal gas_density = _fs->gasDensity(p, T, fsp);

  const DualReal density =
      _ncg_fp->rho_from_p_T(Z * p, T) + _water_fp->rho_from_p_T(_water_fp->vaporPressure(T), T);

  ABS_TEST(gas_density, density, 1.0e-12);
}

/*
 * Verify calculation of gas density, viscosity, enthalpy and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, gasProperties)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Gas region
  DualReal Z = 0.995;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density, viscosity and enthalpy
  _fs->gasProperties(p, T, fsp);
  DualReal gas_density = fsp[1].density;
  DualReal gas_viscosity = fsp[1].viscosity;
  DualReal gas_enthalpy = fsp[1].enthalpy;

  DualReal density =
      _ncg_fp->rho_from_p_T(Z * p, T) + _water_fp->rho_from_p_T(_water_fp->vaporPressure(T), T);
  DualReal viscosity = Z * _ncg_fp->mu_from_p_T(Z * p, T) +
                       (1.0 - Z) * _water_fp->mu_from_p_T(_water_fp->vaporPressure(T), T);
  DualReal enthalpy = Z * _ncg_fp->h_from_p_T(Z * p, T) +
                      (1.0 - Z) * _water_fp->h_from_p_T(_water_fp->vaporPressure(T), T);

  ABS_TEST(gas_density, density, 1.0e-10);
  ABS_TEST(gas_viscosity, viscosity, 1.0e-10);
  ABS_TEST(gas_enthalpy, enthalpy, 1.0e-10);

  // Verify derivatives
  Real ddensity_dp = gas_density.derivatives()[_pidx];
  Real ddensity_dT = gas_density.derivatives()[_Tidx];
  Real ddensity_dZ = gas_density.derivatives()[_Zidx];
  Real dviscosity_dp = gas_viscosity.derivatives()[_pidx];
  Real dviscosity_dT = gas_viscosity.derivatives()[_Tidx];
  Real dviscosity_dZ = gas_viscosity.derivatives()[_Zidx];
  Real denthalpy_dp = gas_enthalpy.derivatives()[_pidx];
  Real denthalpy_dT = gas_enthalpy.derivatives()[_Tidx];
  Real denthalpy_dZ = gas_enthalpy.derivatives()[_Zidx];

  const Real dp = 1.0e-1;
  _fs->gasProperties(p + dp, T, fsp);
  DualReal rho1 = fsp[1].density;
  DualReal mu1 = fsp[1].viscosity;
  DualReal h1 = fsp[1].enthalpy;

  _fs->gasProperties(p - dp, T, fsp);
  DualReal rho2 = fsp[1].density;
  DualReal mu2 = fsp[1].viscosity;
  DualReal h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2).value() / (2.0 * dp), 1.0e-7);
  REL_TEST(dviscosity_dp, (mu1 - mu2).value() / (2.0 * dp), 1.0e-7);
  REL_TEST(denthalpy_dp, (h1 - h2).value() / (2.0 * dp), 5.0e-7);

  const Real dT = 1.0e-3;
  _fs->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fs->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2).value() / (2.0 * dT), 1.0e-7);
  REL_TEST(dviscosity_dT, (mu1 - mu2).value() / (2.0 * dT), 1.0e-8);
  REL_TEST(denthalpy_dT, (h1 - h2).value() / (2.0 * dT), 1.0e-8);

  // Note: mass fraction changes with Z
  const Real dZ = 1.0e-8;
  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dZ, (rho1 - rho2).value() / (2.0 * dZ), 5.0e-8);
  REL_TEST(dviscosity_dZ, (mu1 - mu2).value() / (2.0 * dZ), 5.0e-8);
  REL_TEST(denthalpy_dZ, (h1 - h2).value() / (2.0 * dZ), 1.0e-8);

  // Check derivatives in the two phase region as well. Note that the mass fractions
  // vary with pressure and temperature in this region
  Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fs->gasProperties(p, T, fsp);
  gas_density = fsp[1].density;
  gas_viscosity = fsp[1].viscosity;
  gas_enthalpy = fsp[1].enthalpy;
  ddensity_dp = gas_density.derivatives()[_pidx];
  ddensity_dT = gas_density.derivatives()[_Tidx];
  ddensity_dZ = gas_density.derivatives()[_Zidx];
  dviscosity_dp = gas_viscosity.derivatives()[_pidx];
  dviscosity_dT = gas_viscosity.derivatives()[_Tidx];
  dviscosity_dZ = gas_viscosity.derivatives()[_Zidx];
  denthalpy_dp = gas_enthalpy.derivatives()[_pidx];
  denthalpy_dT = gas_enthalpy.derivatives()[_Tidx];
  denthalpy_dZ = gas_enthalpy.derivatives()[_Zidx];

  _fs->massFractions(p + dp, T, Z, phase_state, fsp);
  _fs->gasProperties(p + dp, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fs->massFractions(p - dp, T, Z, phase_state, fsp);
  _fs->gasProperties(p - dp, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2).value() / (2.0 * dp), 1.0e-7);
  REL_TEST(dviscosity_dp, (mu1 - mu2).value() / (2.0 * dp), 1.0e-7);
  REL_TEST(denthalpy_dp, (h1 - h2).value() / (2.0 * dp), 1.0e-7);

  _fs->massFractions(p, T + dT, Z, phase_state, fsp);
  _fs->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fs->massFractions(p, T - dT, Z, phase_state, fsp);
  _fs->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2).value() / (2.0 * dT), 1.0e-7);
  REL_TEST(dviscosity_dT, (mu1 - mu2).value() / (2.0 * dT), 1.0e-8);
  REL_TEST(denthalpy_dT, (h1 - h2).value() / (2.0 * dT), 1.0e-8);

  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  ABS_TEST(ddensity_dZ, (rho1 - rho2).value() / (2.0 * dZ), 1.0e-8);
  ABS_TEST(dviscosity_dT, (mu1 - mu2).value() / (2.0 * dZ), 1.0e-7);
  ABS_TEST(denthalpy_dZ, (h1 - h2).value() / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of liquid density and derivatives
 */
TEST_F(PorousFlowWaterNCGTest, liquidDensity)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  const DualReal liquid_density = _fs->liquidDensity(p, T);

  const DualReal density = _water_fp->rho_from_p_T(p, T);
  ABS_TEST(liquid_density, density, 1.0e-12);
}

/*
 * Verify calculation of liquid density, viscosity, enthalpy and derivatives. Note that as
 * density and viscosity don't depend on mass fraction, only the liquid region needs to be
 * tested (the calculations are identical in the two phase region). The enthalpy does depend
 * on mass fraction, so should be tested in the two phase region as well as the liquid region
 */
TEST_F(PorousFlowWaterNCGTest, liquidProperties)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // Verify fluid density and viscosity
  _fs->liquidProperties(p, T, fsp);
  DualReal liquid_density = fsp[0].density;
  DualReal liquid_viscosity = fsp[0].viscosity;
  DualReal liquid_enthalpy = fsp[0].enthalpy;

  DualReal density = _water_fp->rho_from_p_T(p, T);
  DualReal viscosity = _water_fp->mu_from_p_T(p, T);
  DualReal enthalpy = _water_fp->h_from_p_T(p, T);

  ABS_TEST(liquid_density, density, 1.0e-12);
  ABS_TEST(liquid_viscosity, viscosity, 1.0e-12);
  ABS_TEST(liquid_enthalpy, enthalpy, 1.0e-12);

  // Verify derivatives
  Real ddensity_dp = liquid_density.derivatives()[_pidx];
  Real ddensity_dT = liquid_density.derivatives()[_Tidx];
  Real ddensity_dZ = liquid_density.derivatives()[_Zidx];
  Real dviscosity_dp = liquid_viscosity.derivatives()[_pidx];
  Real dviscosity_dT = liquid_viscosity.derivatives()[_Tidx];
  Real dviscosity_dZ = liquid_viscosity.derivatives()[_Zidx];
  Real denthalpy_dp = liquid_enthalpy.derivatives()[_pidx];
  Real denthalpy_dT = liquid_enthalpy.derivatives()[_Tidx];
  Real denthalpy_dZ = liquid_enthalpy.derivatives()[_Zidx];

  const Real dp = 10.0;
  _fs->liquidProperties(p + dp, T, fsp);
  DualReal rho1 = fsp[0].density;
  DualReal mu1 = fsp[0].viscosity;
  DualReal h1 = fsp[0].enthalpy;

  _fs->liquidProperties(p - dp, T, fsp);
  DualReal rho2 = fsp[0].density;
  DualReal mu2 = fsp[0].viscosity;
  DualReal h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-6);

  const Real dT = 1.0e-4;
  _fs->liquidProperties(p, T + dT, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fs->liquidProperties(p, T - dT, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-8);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-8);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-8);

  DualReal Z = 0.0001;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  const Real dZ = 1.0e-8;

  _fs->massFractions(p, T, Z, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  denthalpy_dZ = fsp[0].enthalpy.derivatives()[_Zidx];

  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  h1 = fsp[0].enthalpy;

  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-8);

  // Density and viscosity don't depend on Z, so derivatives should be 0
  ABS_TEST(ddensity_dZ, 0.0, 1.0e-12);
  ABS_TEST(dviscosity_dZ, 0.0, 1.0e-12);

  // Check enthalpy calculations in the two phase region as well. Note that the mass fractions
  // vary with pressure and temperature in this region
  Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  denthalpy_dp = fsp[0].enthalpy.derivatives()[_pidx];
  denthalpy_dT = fsp[0].enthalpy.derivatives()[_Tidx];
  denthalpy_dZ = fsp[0].enthalpy.derivatives()[_Zidx];

  _fs->massFractions(p + dp, T, Z, phase_state, fsp);
  _fs->liquidProperties(p + dp, T, fsp);
  h1 = fsp[0].enthalpy;

  _fs->massFractions(p - dp, T, Z, phase_state, fsp);
  _fs->liquidProperties(p - dp, T, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-8);

  _fs->massFractions(p, T + dT, Z, phase_state, fsp);
  _fs->liquidProperties(p, T + dT, fsp);
  h1 = fsp[0].enthalpy;

  _fs->massFractions(p, T - dT, Z, phase_state, fsp);
  _fs->liquidProperties(p, T - dT, fsp);
  h2 = fsp[0].enthalpy;

  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-8);

  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  h1 = fsp[0].enthalpy;

  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, fsp);
  h2 = fsp[0].enthalpy;

  ABS_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowWaterNCGTest, saturation)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  // In the two-phase region, the mass fractions are the equilibrium values, so
  // a temporary value of Z can be used (as long as it corresponds to the two-phase
  // region)
  DualReal Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate Z that gives a saturation of 0.25
  DualReal target_gas_saturation = 0.25;
  fsp[1].saturation = target_gas_saturation;

  // Calculate gas density and liquid density
  _fs->gasProperties(p, T, fsp);
  _fs->liquidProperties(p, T, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  DualReal Zc =
      (target_gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
       (1.0 - target_gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
      (target_gas_saturation * fsp[1].density + (1.0 - target_gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  DualReal gas_saturation = _fs->saturation(p, T, Zc, fsp);

  REL_TEST(gas_saturation, target_gas_saturation, 1.0e-6);

  // Test the derivatives of gas saturation
  gas_saturation = _fs->saturation(p, T, Z, fsp);
  const Real dp = 1.0e-1;

  Real dgas_saturation_dp = gas_saturation.derivatives()[_pidx];
  Real dgas_saturation_dT = gas_saturation.derivatives()[_Tidx];
  Real dgas_saturation_dZ = gas_saturation.derivatives()[_Zidx];

  _fs->massFractions(p + dp, T, Z, phase_state, fsp);
  DualReal gsat1 = _fs->saturation(p + dp, T, Z, fsp);

  _fs->massFractions(p - dp, T, Z, phase_state, fsp);
  DualReal gsat2 = _fs->saturation(p - dp, T, Z, fsp);

  REL_TEST(dgas_saturation_dp, (gsat1 - gsat2).value() / (2.0 * dp), 1.0e-7);

  const Real dT = 1.0e-4;

  _fs->massFractions(p, T + dT, Z, phase_state, fsp);
  gsat1 = _fs->saturation(p, T + dT, Z, fsp);

  _fs->massFractions(p, T - dT, Z, phase_state, fsp);
  gsat2 = _fs->saturation(p, T - dT, Z, fsp);

  REL_TEST(dgas_saturation_dT, (gsat1 - gsat2).value() / (2.0 * dT), 1.0e-7);

  const Real dZ = 1.0e-8;

  _fs->massFractions(p, T, Z + dZ, phase_state, fsp);
  gsat1 = _fs->saturation(p, T, Z + dZ, fsp);

  _fs->massFractions(p, T, Z - dZ, phase_state, fsp);
  gsat2 = _fs->saturation(p, T, Z - dZ, fsp);

  REL_TEST(dgas_saturation_dZ, (gsat1 - gsat2).value() / (2.0 * dZ), 1.0e-7);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowWaterNCGTest, twoPhase)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  DualReal Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  // Dummy qp value that isn't needed to test this
  const unsigned int qp = 0;

  _fs->massFractions(p, T, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fs->twoPhaseProperties(p, T, Z, qp, fsp);
  DualReal liquid_density = fsp[0].density;
  DualReal liquid_viscosity = fsp[0].viscosity;
  DualReal liquid_enthalpy = fsp[0].enthalpy;
  DualReal gas_saturation = fsp[1].saturation;
  DualReal gas_density = fsp[1].density;
  DualReal gas_viscosity = fsp[1].viscosity;
  DualReal gas_enthalpy = fsp[1].enthalpy;

  // As all the values are provided by the gasProperties(), liquidProperties() and
  // saturation(), we can simply compare the DualReal results are the same (really
  // only need to make sure that liquid properties are calculated correctly given the saturation)
  std::vector<FluidStateProperties> fsp2(2, FluidStateProperties(2));

  _fs->massFractions(p, T, Z, phase_state, fsp2);
  _fs->gasProperties(p, T, fsp2);
  fsp2[1].saturation = _fs->saturation(p, T, Z, fsp2);

  const DualReal pliq = p - _pc->capillaryPressure(1.0 - fsp2[1].saturation, qp);
  _fs->liquidProperties(pliq, T, fsp2);

  ABS_TEST(gas_density, fsp2[1].density, 1.0e-12);
  ABS_TEST(gas_viscosity, fsp2[1].viscosity, 1.0e-12);
  ABS_TEST(gas_enthalpy, fsp2[1].enthalpy, 1.0e-12);

  ABS_TEST(liquid_density, fsp2[0].density, 1.0e-12);
  ABS_TEST(liquid_viscosity, fsp2[0].viscosity, 1.0e-12);
  ABS_TEST(liquid_enthalpy, fsp2[0].enthalpy, 1.0e-12);
}

/*
 * Verify calculation of total mass fraction given a gas saturation
 */
TEST_F(PorousFlowWaterNCGTest, totalMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real s = 0.2;
  const Real Xnacl = 0.1;
  const unsigned qp = 0;

  Real Z = _fs->totalMassFraction(p, T, Xnacl, s, qp);

  // Test that the saturation calculated in this fluid state using Z is equal to s
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(2));

  DualReal pressure = p;
  DualReal temperature = T;
  DualReal z = Z;
  _fs->massFractions(pressure, temperature, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  DualReal gas_saturation = _fs->saturation(pressure, temperature, z, fsp);
  REL_TEST(gas_saturation, s, 1.0e-5);
}

/*
 * Verify calculation of enthalpy of dissolution. Note: the values calculated compare
 * well by eye to the values presented in Figure 4 of Battistelli et al, "A fluid property
 * module for the TOUGH2 simulator for saline brines with non-condensible gas"
 */
TEST_F(PorousFlowWaterNCGTest, enthalpyOfDissolution)
{
  // T = 50C
  DualReal T = 323.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  // Enthalpy of dissolution of NCG in water
  DualReal hdis = _fs->enthalpyOfDissolution(T);
  REL_TEST(hdis.value(), -3.45731e5, 1.0e-5);

  // T = 350C
  T = 623.15;
  Moose::derivInsert(T.derivatives(), 2, 1.0);

  // Enthalpy of dissolution of NCG in water
  hdis = _fs->enthalpyOfDissolution(T);
  REL_TEST(hdis.value(), 1.23423e+06, 1.0e-5);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  DualReal hdis2 = _fs->enthalpyOfDissolution(T + dT);
  DualReal hdis3 = _fs->enthalpyOfDissolution(T - dT);

  REL_TEST(hdis.derivatives()[_Tidx], (hdis2 - hdis3) / (2.0 * dT), 1.0e-6);
}

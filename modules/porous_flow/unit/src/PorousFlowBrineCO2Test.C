//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrineCO2Test.h"
#include "FluidPropertiesTestUtils.h"

/**
 * Verify that the correct name is supplied
 */
TEST_F(PorousFlowBrineCO2Test, name) { EXPECT_EQ("brine-co2", _fs->fluidStateName()); }

/**
 * Verify that the correct values for the fluid phase and component indices are supplied
 */
TEST_F(PorousFlowBrineCO2Test, indices)
{
  EXPECT_EQ((unsigned int)2, _fs->numPhases());
  EXPECT_EQ((unsigned int)3, _fs->numComponents());
  EXPECT_EQ((unsigned int)0, _fs->aqueousPhaseIndex());
  EXPECT_EQ((unsigned int)1, _fs->gasPhaseIndex());
  EXPECT_EQ((unsigned int)0, _fs->aqueousComponentIndex());
  EXPECT_EQ((unsigned int)1, _fs->gasComponentIndex());
  EXPECT_EQ((unsigned int)2, _fs->saltComponentIndex());
}

/*
 * Verify calculation of the equilibrium constants and their derivatives wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumConstants)
{
  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  const Real dT = 1.0e-6;

  DualReal K0H2O = _fs->equilibriumConstantH2O(T);
  DualReal K0CO2 = _fs->equilibriumConstantCO2(T);

  ABS_TEST(K0H2O.value(), 0.412597711705, 1.0e-10);
  ABS_TEST(K0CO2.value(), 74.0435888596, 1.0e-10);

  Real K0H2O_2 = _fs->equilibriumConstantH2O(T + dT).value();
  Real K0CO2_2 = _fs->equilibriumConstantCO2(T + dT).value();

  Real dK0H2O_dT_fd = (K0H2O_2 - K0H2O.value()) / dT;
  Real dK0CO2_dT_fd = (K0CO2_2 - K0CO2.value()) / dT;
  REL_TEST(K0H2O.derivatives()[_Tidx], dK0H2O_dT_fd, 1.0e-6);
  REL_TEST(K0CO2.derivatives()[_Tidx], dK0CO2_dT_fd, 1.0e-6);

  // Test the high temperature formulation
  T = 450.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  K0H2O = _fs->equilibriumConstantH2O(T);
  K0CO2 = _fs->equilibriumConstantCO2(T);

  ABS_TEST(K0H2O.value(), 8.75944517916, 1.0e-10);
  ABS_TEST(K0CO2.value(), 105.013867434, 1.0e-10);

  K0H2O_2 = _fs->equilibriumConstantH2O(T + dT).value();
  K0CO2_2 = _fs->equilibriumConstantCO2(T + dT).value();

  dK0H2O_dT_fd = (K0H2O_2 - K0H2O.value()) / dT;
  dK0CO2_dT_fd = (K0CO2_2 - K0CO2.value()) / dT;
  REL_TEST(K0H2O.derivatives()[_Tidx], dK0H2O_dT_fd, 1.0e-6);
  REL_TEST(K0CO2.derivatives()[_Tidx], dK0CO2_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of the fugacity coefficients
 */
TEST_F(PorousFlowBrineCO2Test, fugacityCoefficients)
{
  // Test the low temperature formulation
  DualReal p = 40.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.0;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  DualReal co2_density = _co2_fp->rho_from_p_T(p, T);

  DualReal phiH2O, phiCO2;
  _fs->fugacityCoefficientsLowTemp(p, T, co2_density, phiCO2, phiH2O);
  ABS_TEST(phiCO2.value(), 0.401939386415, 1.0e-8);
  ABS_TEST(phiH2O.value(), 0.0898968578757, 1.0e-8);

  // Test the high temperature formulation
  T = 423.15;

  DualReal xco2, yh2o;
  co2_density = _co2_fp->rho_from_p_T(p, T);

  _fs->solveEquilibriumMoleFractionHighTemp(
      p.value(), T.value(), Xnacl.value(), co2_density.value(), xco2.value(), yh2o.value());
  phiH2O = _fs->fugacityCoefficientH2OHighTemp(p, T, co2_density, xco2, yh2o);
  phiCO2 = _fs->fugacityCoefficientCO2HighTemp(p, T, co2_density, xco2, yh2o);

  ABS_TEST(phiH2O.value(), 0.156303437579, 1.0e-8);
  ABS_TEST(phiCO2.value(), 0.641936639599, 1.0e-8);

  // Test that the same results are returned in fugacityCoefficientsHighTemp()
  DualReal phiCO2_2, phiH2O_2;
  _fs->fugacityCoefficientsHighTemp(p, T, co2_density, xco2, yh2o, phiCO2_2, phiH2O_2);

  ABS_TEST(phiH2O, phiH2O_2, 1.0e-12);
  ABS_TEST(phiCO2, phiCO2_2, 1.0e-12);
}

/*
 * Verify calculation of the H2O and CO2 activity coefficients
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficients)
{
  DualReal xco2 = 0.01;
  DualReal T = 350.0;

  DualReal gammaH2O = _fs->activityCoefficientH2O(T, xco2);
  DualReal gammaCO2 = _fs->activityCoefficientCO2(T, xco2);

  ABS_TEST(gammaH2O.value(), 1.0, 1.0e-10);
  ABS_TEST(gammaCO2.value(), 1.0, 1.0e-10);

  T = 450.0;

  gammaH2O = _fs->activityCoefficientH2O(T, xco2);
  gammaCO2 = _fs->activityCoefficientCO2(T, xco2);

  ABS_TEST(gammaH2O.value(), 1.00022113664, 1.0e-10);
  ABS_TEST(gammaCO2.value(), 0.956736800255, 1.0e-10);
}

/*
 * Verify calculation of the Duan and Sun activity coefficient and its derivatives wrt
 * pressure, temperature and salt mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficientCO2Brine)
{
  DualReal p = 10.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  // Low temperature regime
  DualReal gamma = _fs->activityCoefficient(p, T, Xnacl);
  ABS_TEST(gamma.value(), 1.43276649338, 1.0e-8);

  DualReal gamma_2 = _fs->activityCoefficient(p + dp, T, Xnacl);

  Real dgamma_dp_fd = (gamma_2.value() - gamma.value()) / dp;
  REL_TEST(gamma.derivatives()[_pidx], dgamma_dp_fd, 1.0e-6);

  gamma_2 = _fs->activityCoefficient(p, T + dT, Xnacl);

  Real dgamma_dT_fd = (gamma_2.value() - gamma.value()) / dT;
  REL_TEST(gamma.derivatives()[_Tidx], dgamma_dT_fd, 1.0e-6);

  gamma_2 = _fs->activityCoefficient(p, T, Xnacl + dx);

  Real dgamma_dX_fd = (gamma_2.value() - gamma.value()) / dx;
  REL_TEST(gamma.derivatives()[_Xidx], dgamma_dX_fd, 1.0e-6);

  // High temperature regime
  T = 523.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  gamma = _fs->activityCoefficientHighTemp(T, Xnacl);
  ABS_TEST(gamma.value(), 1.50047006243, 1.0e-8);

  gamma_2 = _fs->activityCoefficientHighTemp(T + dT, Xnacl);
  dgamma_dT_fd = (gamma_2.value() - gamma.value()) / dT;
  REL_TEST(gamma.derivatives()[_Tidx], dgamma_dT_fd, 1.0e-6);

  gamma_2 = _fs->activityCoefficientHighTemp(T, Xnacl + dx);
  dgamma_dX_fd = (gamma_2.value() - gamma.value()) / dx;
  REL_TEST(gamma.derivatives()[_Xidx], dgamma_dX_fd, 1.0e-6);

  // Check that both formulations return gamma = 1 for Xnacl = 0
  Xnacl = 0.0;
  T = 350.0;
  gamma = _fs->activityCoefficient(p, T, Xnacl);
  ABS_TEST(gamma.value(), 1.0, 1.0e-12);

  T = 523.15;
  gamma = _fs->activityCoefficientHighTemp(T, Xnacl);
  ABS_TEST(gamma.value(), 1.0, 1.0e-12);
}

/*
 * Verify calculation of the partial density of CO2 and its derivative wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, partialDensity)
{
  DualReal T = 473.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  const Real dT = 1.0e-6;

  DualReal partial_density = _fs->partialDensityCO2(T);
  ABS_TEST(partial_density.value(), 893.332, 1.0e-3);

  DualReal partial_density_2 = _fs->partialDensityCO2(T + dT);

  Real dpartial_density_dT_fd = (partial_density_2.value() - partial_density.value()) / dT;
  REL_TEST(partial_density.derivatives()[_Tidx], dpartial_density_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of equilibrium mass fraction and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMassFraction)
{
  // Low temperature regime
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  DualReal X, Y;
  _fs->equilibriumMassFractions(p, T, Xnacl, X, Y);

  ABS_TEST(X.value(), 0.0035573020148, 1.0e-10);
  ABS_TEST(Y.value(), 0.0171977397214, 1.0e-10);

  // Derivative wrt pressure
  DualReal X1, Y1, X2, Y2;
  _fs->equilibriumMassFractions(p - dp, T, Xnacl, X1, Y1);
  _fs->equilibriumMassFractions(p + dp, T, Xnacl, X2, Y2);

  Real dX_dp_fd = (X2.value() - X1.value()) / (2.0 * dp);
  Real dY_dp_fd = (Y2.value() - Y1.value()) / (2.0 * dp);

  REL_TEST(X.derivatives()[_pidx], dX_dp_fd, 1.0e-6);
  REL_TEST(Y.derivatives()[_pidx], dY_dp_fd, 1.0e-6);

  // Derivative wrt temperature
  _fs->equilibriumMassFractions(p, T - dT, Xnacl, X1, Y1);
  _fs->equilibriumMassFractions(p, T + dT, Xnacl, X2, Y2);

  Real dX_dT_fd = (X2.value() - X1.value()) / (2.0 * dT);
  Real dY_dT_fd = (Y2.value() - Y1.value()) / (2.0 * dT);

  REL_TEST(X.derivatives()[_Tidx], dX_dT_fd, 1.0e-6);
  REL_TEST(Y.derivatives()[_Tidx], dY_dT_fd, 1.0e-6);

  // Derivative wrt salt mass fraction
  _fs->equilibriumMassFractions(p, T, Xnacl - dx, X1, Y1);
  _fs->equilibriumMassFractions(p, T, Xnacl + dx, X2, Y2);

  Real dX_dX_fd = (X2.value() - X1.value()) / (2.0 * dx);
  Real dY_dX_fd = (Y2.value() - Y1).value() / (2.0 * dx);

  REL_TEST(X.derivatives()[_Xidx], dX_dX_fd, 1.0e-6);
  REL_TEST(Y.derivatives()[_Xidx], dY_dX_fd, 1.0e-6);

  // High temperature regime
  p = 10.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  T = 525.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  _fs->equilibriumMassFractions(p, T, Xnacl, X, Y);

  ABS_TEST(X.value(), 0.016299479086, 1.0e-10);
  ABS_TEST(Y.value(), 0.249471400766, 1.0e-10);
}

/*
 * Verify calculation of actual mass fraction and derivatives depending on value of
 * total mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, MassFraction)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fs->numPhases();
  const unsigned int nc = _fs->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  DualReal Z = 0.0001;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  DualReal Xco2 = fsp[0].mass_fraction[1];
  DualReal Yco2 = fsp[1].mass_fraction[1];
  DualReal Xh2o = fsp[0].mass_fraction[0];
  DualReal Yh2o = fsp[1].mass_fraction[0];
  DualReal Xnacl2 = fsp[0].mass_fraction[2];
  ABS_TEST(Xco2.value(), Z.value(), 1.0e-8);
  ABS_TEST(Yco2.value(), 0.0, 1.0e-8);
  ABS_TEST(Xh2o.value(), 1.0 - Z.value(), 1.0e-8);
  ABS_TEST(Yh2o.value(), 0.0, 1.0e-8);
  ABS_TEST(Xnacl2.value(), Xnacl.value(), 1.0e-8);

  // Verify derivatives
  ABS_TEST(Xco2.derivatives()[_pidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Tidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Xidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Zidx], 1.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_pidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Tidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Xidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Zidx], 0.0, 1.0e-8);
  ABS_TEST(Xnacl.derivatives()[_Xidx], 1.0, 1.0e-8);

  // Gas region
  Z = 0.995;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  DualReal Ynacl = fsp[1].mass_fraction[2];
  ABS_TEST(Xco2.value(), 0.0, 1.0e-8);
  ABS_TEST(Yco2.value(), Z.value(), 1.0e-8);
  ABS_TEST(Xh2o.value(), 0.0, 1.0e-8);
  ABS_TEST(Yh2o.value(), 1.0 - Z.value(), 1.0e-8);
  ABS_TEST(Ynacl.value(), 0.0, 1.0e-8);

  // Verify derivatives
  ABS_TEST(Xco2.derivatives()[_pidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Tidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Xidx], 0.0, 1.0e-8);
  ABS_TEST(Xco2.derivatives()[_Zidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_pidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Tidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Xidx], 0.0, 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Zidx], 1.0, 1.0e-8);
  ABS_TEST(Ynacl.derivatives()[_Xidx], 0.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  // be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  DualReal Xco2_eq, Yh2o_eq;
  _fs->equilibriumMassFractions(p, T, Xnacl, Xco2_eq, Yh2o_eq);

  // Verfify mass fraction values - comparing DualReals verifies that derivatives
  // are also identical
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xco2, Xco2_eq, 1.0e-8);
  ABS_TEST(Yco2, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST(Xh2o, 1.0 - Xco2_eq, 1.0e-8);
  ABS_TEST(Yh2o, Yh2o_eq, 1.0e-8);

  // Use finite differences to verify derivative wrt Z is unaffected by Z
  const Real dZ = 1.0e-8;
  _fs->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  DualReal Xco21 = fsp[0].mass_fraction[1];
  DualReal Yco21 = fsp[1].mass_fraction[1];
  _fs->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  DualReal Xco22 = fsp[0].mass_fraction[1];
  DualReal Yco22 = fsp[1].mass_fraction[1];

  ABS_TEST(Xco2.derivatives()[_Zidx], (Xco21.value() - Xco22.value()) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(Yco2.derivatives()[_Zidx], (Yco21.value() - Yco22.value()) / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of gas density, viscosity enthalpy, and derivatives. Note that as
 * these properties don't depend on mass fraction, only the gas region needs to be
 * tested (the calculations are identical in the two phase region)
 */
TEST_F(PorousFlowBrineCO2Test, gasProperties)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fs->numPhases();
  const unsigned int nc = _fs->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Gas region
  DualReal Z = 0.995;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density, viscosity and enthalpy
  _fs->gasProperties(p, T, fsp);
  DualReal gas_density = fsp[1].density;
  DualReal gas_viscosity = fsp[1].viscosity;
  DualReal gas_enthalpy = fsp[1].enthalpy;

  Real density = _co2_fp->rho_from_p_T(p.value(), T.value());
  Real viscosity = _co2_fp->mu_from_p_T(p.value(), T.value());
  Real enthalpy = _co2_fp->h_from_p_T(p.value(), T.value());

  ABS_TEST(gas_density.value(), density, 1.0e-8);
  ABS_TEST(gas_viscosity.value(), viscosity, 1.0e-8);
  ABS_TEST(gas_enthalpy.value(), enthalpy, 1.0e-8);

  // Verify derivatives
  const Real dp = 1.0e-1;
  _fs->gasProperties(p + dp, T, fsp);
  Real rho1 = fsp[1].density.value();
  Real mu1 = fsp[1].viscosity.value();
  Real h1 = fsp[1].enthalpy.value();

  _fs->gasProperties(p - dp, T, fsp);
  Real rho2 = fsp[1].density.value();
  Real mu2 = fsp[1].viscosity.value();
  Real h2 = fsp[1].enthalpy.value();

  REL_TEST(gas_density.derivatives()[_pidx], (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(gas_viscosity.derivatives()[_pidx], (mu1 - mu2) / (2.0 * dp), 1.0e-6);
  REL_TEST(gas_enthalpy.derivatives()[_pidx], (h1 - h2) / (2.0 * dp), 1.0e-6);

  const Real dT = 1.0e-3;
  _fs->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density.value();
  mu1 = fsp[1].viscosity.value();
  h1 = fsp[1].enthalpy.value();

  _fs->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density.value();
  mu2 = fsp[1].viscosity.value();
  h2 = fsp[1].enthalpy.value();

  REL_TEST(gas_density.derivatives()[_Tidx], (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(gas_viscosity.derivatives()[_Tidx], (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(gas_enthalpy.derivatives()[_Tidx], (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Note: mass fraction changes with Z
  const Real dZ = 1.0e-8;
  _fs->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho1 = fsp[1].density.value();
  mu1 = fsp[1].viscosity.value();
  h1 = fsp[1].enthalpy.value();

  _fs->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fs->gasProperties(p, T, fsp);
  rho2 = fsp[1].density.value();
  mu2 = fsp[1].viscosity.value();
  h2 = fsp[1].enthalpy.value();

  ABS_TEST(gas_density.derivatives()[_Zidx], (rho1 - rho2) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(gas_viscosity.derivatives()[_Zidx], (mu1 - mu2) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(gas_enthalpy.derivatives()[_Zidx], (h1 - h2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of liquid density, viscosity, enthalpy, and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, liquidProperties)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fs->numPhases();
  const unsigned int nc = _fs->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  DualReal Z = 0.0001;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verify fluid density and viscosity
  _fs->liquidProperties(p, T, Xnacl, fsp);

  DualReal liquid_density = fsp[0].density;
  DualReal liquid_viscosity = fsp[0].viscosity;
  DualReal liquid_enthalpy = fsp[0].enthalpy;

  Real co2_partial_density = _fs->partialDensityCO2(T).value();
  Real brine_density = _brine_fp->rho_from_p_T_X(p.value(), T.value(), Xnacl.value());

  Real density = 1.0 / (Z.value() / co2_partial_density + (1.0 - Z.value()) / brine_density);

  Real viscosity = _brine_fp->mu_from_p_T_X(p.value(), T.value(), Xnacl.value());

  Real brine_enthalpy = _brine_fp->h_from_p_T_X(p.value(), T.value(), Xnacl.value());
  Real hdis = _fs->enthalpyOfDissolution(T).value();
  Real co2_enthalpy = _co2_fp->h_from_p_T(p.value(), T.value());
  Real enthalpy = (1.0 - Z.value()) * brine_enthalpy + Z.value() * (co2_enthalpy + hdis);

  ABS_TEST(liquid_density.value(), density, 1.0e-12);
  ABS_TEST(liquid_viscosity.value(), viscosity, 1.0e-12);
  ABS_TEST(liquid_enthalpy.value(), enthalpy, 1.0e-12);

  // Verify derivatives
  // Derivatives wrt pressure
  const Real dp = 1.0;
  _fs->liquidProperties(p + dp, T, Xnacl, fsp);
  Real rho1 = fsp[0].density.value();
  Real mu1 = fsp[0].viscosity.value();
  Real h1 = fsp[0].enthalpy.value();

  _fs->liquidProperties(p - dp, T, Xnacl, fsp);
  Real rho2 = fsp[0].density.value();
  Real mu2 = fsp[0].viscosity.value();
  Real h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_pidx], (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_pidx], (mu1 - mu2) / (2.0 * dp), 2.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_pidx], (h1 - h2) / (2.0 * dp), 1.0e-6);

  // Derivatives wrt temperature
  const Real dT = 1.0e-4;
  _fs->liquidProperties(p, T + dT, Xnacl, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->liquidProperties(p, T - dT, Xnacl, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_Tidx], (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_Tidx], (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_Tidx], (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt Xnacl
  const Real dx = 1.0e-8;
  _fs->liquidProperties(p, T, Xnacl + dx, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->liquidProperties(p, T, Xnacl - dx, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_Xidx], (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_Xidx], (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_Xidx], (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt Z
  const Real dZ = 1.0e-8;
  _fs->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_Zidx], (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(liquid_viscosity.derivatives()[_Zidx], (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_Zidx], (h1 - h2) / (2.0 * dZ), 1.0e-6);

  // Two-phase region
  Z = 0.045;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Verify fluid density and viscosity derivatives
  _fs->liquidProperties(p, T, Xnacl, fsp);

  liquid_density = fsp[0].density;
  liquid_viscosity = fsp[0].viscosity;
  liquid_enthalpy = fsp[0].enthalpy;

  // Derivatives wrt pressure
  _fs->massFractions(p + dp, T, Xnacl, Z, phase_state, fsp);
  _fs->liquidProperties(p + dp, T, Xnacl, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->massFractions(p - dp, T, Xnacl, Z, phase_state, fsp);
  _fs->liquidProperties(p - dp, T, Xnacl, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_pidx], (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_pidx], (mu1 - mu2) / (2.0 * dp), 2.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_pidx], (h1 - h2) / (2.0 * dp), 1.0e-6);

  // Derivatives wrt temperature
  _fs->massFractions(p, T + dT, Xnacl, Z, phase_state, fsp);
  _fs->liquidProperties(p, T + dT, Xnacl, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->massFractions(p, T - dT, Xnacl, Z, phase_state, fsp);
  _fs->liquidProperties(p, T - dT, Xnacl, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_Tidx], (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_Tidx], (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_Tidx], (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt Xnacl
  _fs->massFractions(p, T, Xnacl + dx, Z, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl + dx, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->massFractions(p, T, Xnacl - dx, Z, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl - dx, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  REL_TEST(liquid_density.derivatives()[_Xidx], (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST(liquid_viscosity.derivatives()[_Xidx], (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST(liquid_enthalpy.derivatives()[_Xidx], (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt Z
  _fs->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl, fsp);
  rho1 = fsp[0].density.value();
  mu1 = fsp[0].viscosity.value();
  h1 = fsp[0].enthalpy.value();

  _fs->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fs->liquidProperties(p, T, Xnacl, fsp);
  rho2 = fsp[0].density.value();
  mu2 = fsp[0].viscosity.value();
  h2 = fsp[0].enthalpy.value();

  ABS_TEST(liquid_density.derivatives()[_Zidx], (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(liquid_viscosity.derivatives()[_Zidx], (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(liquid_enthalpy.derivatives()[_Zidx], (h1 - h2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowBrineCO2Test, saturation)
{
  DualReal p = 1.0e6;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);

  DualReal T = 350.0;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fs->numPhases();
  const unsigned int nc = _fs->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // In the two-phase region, the mass fractions are the equilibrium values, so
  // a temporary value of Z can be used (as long as it corresponds to the two-phase
  // region)
  DualReal Z = 0.45;
  Moose::derivInsert(Z.derivatives(), _Zidx, 1.0);

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate Z that gives a saturation of 0.25
  DualReal gas_saturation = 0.25;
  DualReal liquid_pressure = p - _pc->capillaryPressure(1.0 - gas_saturation);

  // Calculate gas density and liquid density
  _fs->gasProperties(p, T, fsp);
  _fs->liquidProperties(liquid_pressure, T, Xnacl, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  DualReal Zc = (gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
                 (1.0 - gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
                (gas_saturation * fsp[1].density + (1.0 - gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  DualReal saturation = _fs->saturation(p, T, Xnacl, Zc, fsp);
  ABS_TEST(saturation.value(), gas_saturation.value(), 1.0e-6);

  // Test the derivatives of gas saturation
  gas_saturation = _fs->saturation(p, T, Xnacl, Z, fsp);

  const Real dp = 1.0e-1;

  // Derivative wrt pressure
  _fs->massFractions(p + dp, T, Xnacl, Z, phase_state, fsp);
  Real gsat1 = _fs->saturation(p + dp, T, Xnacl, Z, fsp).value();

  _fs->massFractions(p - dp, T, Xnacl, Z, phase_state, fsp);
  Real gsat2 = _fs->saturation(p - dp, T, Xnacl, Z, fsp).value();

  REL_TEST(gas_saturation.derivatives()[_pidx], (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt temperature
  const Real dT = 1.0e-4;
  _fs->massFractions(p, T + dT, Xnacl, Z, phase_state, fsp);
  gsat1 = _fs->saturation(p, T + dT, Xnacl, Z, fsp).value();

  _fs->massFractions(p, T - dT, Xnacl, Z, phase_state, fsp);
  gsat2 = _fs->saturation(p, T - dT, Xnacl, Z, fsp).value();

  REL_TEST(gas_saturation.derivatives()[_Tidx], (gsat1 - gsat2) / (2.0 * dT), 1.0e-6);

  // Derivative wrt Xnacl
  const Real dx = 1.0e-8;
  _fs->massFractions(p, T, Xnacl + dx, Z, phase_state, fsp);
  gsat1 = _fs->saturation(p, T, Xnacl + dx, Z, fsp).value();

  _fs->massFractions(p, T, Xnacl - dx, Z, phase_state, fsp);
  gsat2 = _fs->saturation(p, T, Xnacl - dx, Z, fsp).value();

  REL_TEST(gas_saturation.derivatives()[_Xidx], (gsat1 - gsat2) / (2.0 * dx), 1.0e-6);

  // Derivative wrt Z
  const Real dZ = 1.0e-8;

  _fs->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  gsat1 = _fs->saturation(p, T, Xnacl, Z + dZ, fsp).value();

  gsat2 = _fs->saturation(p, T, Xnacl, Z - dZ, fsp).value();

  REL_TEST(gas_saturation.derivatives()[_Zidx], (gsat1 - gsat2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of total mass fraction given a gas saturation
 */
TEST_F(PorousFlowBrineCO2Test, totalMassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real Xnacl = 0.1;
  const Real s = 0.2;
  const unsigned qp = 0;

  Real Z = _fs->totalMassFraction(p, T, Xnacl, s, qp);

  // Test that the saturation calculated in this fluid state using Z is equal to s
  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fs->numPhases();
  const unsigned int nc = _fs->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  DualReal pressure = p;
  DualReal temperature = T;
  DualReal z = Z;
  DualReal x = Xnacl;
  _fs->massFractions(pressure, temperature, x, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  DualReal gas_saturation = _fs->saturation(pressure, temperature, x, z, fsp);
  ABS_TEST(gas_saturation, s, 1.0e-6);
}

/*
 * Verify calculation of Henry constant with brine correction. Note: the values
 * calculated compare well by eye to the values presented in Figure 4 of
 * Battistelli et al, "A fluid property module for the TOUGH2 simulator for saline
 * brines with non-condensible gas"
 */
TEST_F(PorousFlowBrineCO2Test, henryConstant)
{
  DualReal T = 373.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  DualReal Kh = _fs->henryConstant(T, Xnacl);
  REL_TEST(Kh.value(), 7.46559e+08, 1.0e-3);

  T = 473.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  Xnacl = 0.2;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  Kh = _fs->henryConstant(T, Xnacl);
  REL_TEST(Kh.value(), 1.66069e+09, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  DualReal Kh1 = _fs->henryConstant(T + dT, Xnacl);
  DualReal Kh2 = _fs->henryConstant(T - dT, Xnacl);

  REL_TEST(Kh.derivatives()[_Tidx], (Kh1.value() - Kh2.value()) / (2.0 * dT), 1.0e-6);

  // Test the derivative wrt Xnacl
  const Real dx = 1.0e-8;
  Kh1 = _fs->henryConstant(T, Xnacl + dx);
  Kh2 = _fs->henryConstant(T, Xnacl - dx);

  REL_TEST(Kh.derivatives()[_Xidx], (Kh1.value() - Kh2.value()) / (2.0 * dx), 1.0e-6);
}

/*
 * Verify calculation of enthalpy of dissolution. Note: the values calculated compare
 * well by eye to the values presented in Figure 4 of Battistelli et al, "A fluid property
 * module for the TOUGH2 simulator for saline brines with non-condensible gas"
 */
TEST_F(PorousFlowBrineCO2Test, enthalpyOfDissolutionGas)
{
  // T = 50C
  DualReal T = 323.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  DualReal Xnacl = 0.1;
  Moose::derivInsert(Xnacl.derivatives(), _Xidx, 1.0);

  // Enthalpy of dissolution of CO2 in brine
  DualReal hdis = _fs->enthalpyOfDissolutionGas(T, Xnacl);
  REL_TEST(hdis, -3.20130e5, 1.0e-3);

  // T = 350C
  T = 623.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  // Enthalpy of dissolution of CO2 in brine
  hdis = _fs->enthalpyOfDissolutionGas(T, Xnacl);
  REL_TEST(hdis, 9.83813e+05, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1 = _fs->enthalpyOfDissolutionGas(T + dT, Xnacl).value();
  Real hdis2 = _fs->enthalpyOfDissolutionGas(T - dT, Xnacl).value();

  REL_TEST(hdis.derivatives()[_Tidx], (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);

  // Test the derivative wrt salt mass fraction
  const Real dx = 1.0e-8;
  hdis1 = _fs->enthalpyOfDissolutionGas(T, Xnacl + dx).value();
  hdis2 = _fs->enthalpyOfDissolutionGas(T, Xnacl - dx).value();

  REL_TEST(hdis.derivatives()[_Xidx], (hdis1 - hdis2) / (2.0 * dx), 1.0e-5);
}

/*
 * Verify calculation of enthalpy of dissolution of CO2. Note: the values calculated compare
 * well by eye to the values presented in Figure 6 of Duan and Sun, An improved model
 * calculating CO2 solubility in pure water and aqueous NaCl solutions from 273 to 533 K
 * and from 0 to 2000 bar, Chemical geology, 193, 257--271 (2003)
 */
TEST_F(PorousFlowBrineCO2Test, enthalpyOfDissolution)
{
  // T = 50C
  DualReal T = 323.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  // Enthalpy of dissolution of CO2 in water
  DualReal hdis = _fs->enthalpyOfDissolution(T);
  REL_TEST(hdis.value(), -3.38185e5, 1.0e-3);

  // T = 350C
  T = 623.15;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);

  // Enthalpy of dissolution of CO2 in water
  hdis = _fs->enthalpyOfDissolution(T);
  REL_TEST(hdis.value(), 5.78787e5, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1 = _fs->enthalpyOfDissolution(T + dT).value();
  Real hdis2 = _fs->enthalpyOfDissolution(T - dT).value();

  REL_TEST(hdis.derivatives()[_Tidx], (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);
}

/**
 * Test iterative calculation of equilibrium mass fractions in the elevated
 * temperature regime
 */
TEST_F(PorousFlowBrineCO2Test, solveEquilibriumMoleFractionHighTemp)
{
  Real p = 20.0e6;
  Real T = 473.15;
  Real Xnacl = 0.0;

  Real yh2o, xco2;
  Real co2_density = _co2_fp->rho_from_p_T(p, T);
  _fs->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.161429743509, 1.0e-10);
  ABS_TEST(xco2, 0.0236966821858, 1.0e-10);

  // Mass fraction equivalent to molality of 2
  p = 30.0e6;
  T = 423.15;
  Xnacl = 0.105;

  _fs->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.0468197608955, 1.0e-10);
  ABS_TEST(xco2, 0.0236644599437, 1.0e-10);

  // Mass fraction equivalent to molality of 4
  T = 523.15;
  Xnacl = 0.189;

  co2_density = _co2_fp->rho_from_p_T(p, T);
  _fs->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.253292728991, 1.0e-10);
  ABS_TEST(xco2, 0.0168344513321, 1.0e-10);
}

/**
 * Test calculation of equilibrium mole fractions over entire temperature range
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMoleFractions)
{
  // Test pure water (Xnacl = 0)
  // Low temperature regime
  DualReal p = 20.0e6;
  DualReal T = 323.15;
  DualReal Xnacl = 0.0;

  DualReal x, y;
  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.00696393845155, 1.0e-8);
  ABS_TEST(x.value(), 0.0236554537395, 1.0e-8);

  // Intermediate temperature regime
  T = 373.15;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.0194394631944, 1.0e-8);
  ABS_TEST(x.value(), 0.020195776139, 1.0e-8);

  // High temperature regime
  p = 30.0e6;
  T = 523.15;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.286117195565, 1.0e-8);
  ABS_TEST(x.value(), 0.0409622051253, 1.0e-8);

  // High temperature regime with low pressure
  p = 1.0e6;
  T = 523.15;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 1.0, 1.0e-8);
  ABS_TEST(x.value(), 0.0, 1.0e-8);

  // Test brine (Xnacl = 0.1)
  // Low temperature regime
  p = 20.0e6;
  T = 323.15;
  Xnacl = 0.1;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.00657335846826, 1.0e-8);
  ABS_TEST(x.value(), 0.0152863933134, 1.0e-8);

  // Intermediate temperature regime
  T = 373.15;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.01831360857, 1.0e-8);
  ABS_TEST(x.value(), 0.0132653916293, 1.0e-8);

  // High temperature regime
  p = 30.0e6;
  T = 523.15;

  _fs->equilibriumMoleFractions(p, T, Xnacl, x, y);
  ABS_TEST(y.value(), 0.270258924237, 1.0e-8);
  ABS_TEST(x.value(), 0.0246589135776, 1.0e-8);
}

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
TEST_F(PorousFlowBrineCO2Test, name) { EXPECT_EQ("brine-co2", _fp->fluidStateName()); }

/**
 * Verify that the correct values for the fluid phase and component indices are supplied
 */
TEST_F(PorousFlowBrineCO2Test, indices)
{
  EXPECT_EQ(2, _fp->numPhases());
  EXPECT_EQ(3, _fp->numComponents());
  EXPECT_EQ(0, _fp->aqueousPhaseIndex());
  EXPECT_EQ(1, _fp->gasPhaseIndex());
  EXPECT_EQ(0, _fp->aqueousComponentIndex());
  EXPECT_EQ(1, _fp->gasComponentIndex());
  EXPECT_EQ(2, _fp->saltComponentIndex());
}

/*
 * Verify calculation of the equilibrium constants and their derivatives wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumConstants)
{
  Real T = 350.0;
  const Real dT = 1.0e-6;

  Real K0H2O, dK0H2O_dT, K0CO2, dK0CO2_dT;
  _fp->equilibriumConstantH2O(T, K0H2O, dK0H2O_dT);
  _fp->equilibriumConstantCO2(T, K0CO2, dK0CO2_dT);

  ABS_TEST(K0H2O, 0.412597711705, 1.0e-10);
  ABS_TEST(K0CO2, 74.0435888596, 1.0e-10);

  Real K0H2O_2, dK0H2O_2_dT, K0CO2_2, dK0CO2_2_dT;
  _fp->equilibriumConstantH2O(T + dT, K0H2O_2, dK0H2O_2_dT);
  _fp->equilibriumConstantCO2(T + dT, K0CO2_2, dK0CO2_2_dT);

  Real dK0H2O_dT_fd = (K0H2O_2 - K0H2O) / dT;
  Real dK0CO2_dT_fd = (K0CO2_2 - K0CO2) / dT;
  REL_TEST(dK0H2O_dT, dK0H2O_dT_fd, 1.0e-6);
  REL_TEST(dK0CO2_dT, dK0CO2_dT_fd, 1.0e-6);

  // Test the high temperature formulation
  T = 450.0;

  _fp->equilibriumConstantH2O(T, K0H2O, dK0H2O_dT);
  _fp->equilibriumConstantCO2(T, K0CO2, dK0CO2_dT);

  ABS_TEST(K0H2O, 8.75944517916, 1.0e-10);
  ABS_TEST(K0CO2, 105.013867434, 1.0e-10);

  _fp->equilibriumConstantH2O(T + dT, K0H2O_2, dK0H2O_2_dT);
  _fp->equilibriumConstantCO2(T + dT, K0CO2_2, dK0CO2_2_dT);

  dK0H2O_dT_fd = (K0H2O_2 - K0H2O) / dT;
  dK0CO2_dT_fd = (K0CO2_2 - K0CO2) / dT;
  REL_TEST(dK0H2O_dT, dK0H2O_dT_fd, 1.0e-6);
  REL_TEST(dK0CO2_dT, dK0CO2_dT_fd, 1.0e-5);
}

/*
 * Verify calculation of the fugacity coefficients
 */
TEST_F(PorousFlowBrineCO2Test, fugacityCoefficients)
{
  // Test the low temperature formulation
  Real p = 40.0e6;
  Real T = 350.0;
  Real Xnacl = 0.0;
  Real co2_density, dco2_density_dp, dco2_density_dT;
  _co2_fp->rho_from_p_T(p, T, co2_density, dco2_density_dp, dco2_density_dT);

  Real phiH2O, dphiH2O_dp, dphiH2O_dT;
  Real phiCO2, dphiCO2_dp, dphiCO2_dT;
  _fp->fugacityCoefficientsLowTemp(p,
                                   T,
                                   co2_density,
                                   dco2_density_dp,
                                   dco2_density_dT,
                                   phiCO2,
                                   dphiCO2_dp,
                                   dphiCO2_dT,
                                   phiH2O,
                                   dphiH2O_dp,
                                   dphiH2O_dT);

  ABS_TEST(phiCO2, 0.401938, 1.0e-6);
  ABS_TEST(phiH2O, 0.0898968, 1.0e-6);

  // Test the high temperature formulation
  T = 423.15;

  Real xco2, yh2o;
  co2_density = _co2_fp->rho_from_p_T(p, T);

  _fp->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  phiH2O = _fp->fugacityCoefficientH2OHighTemp(p, T, co2_density, xco2, yh2o);
  phiCO2 = _fp->fugacityCoefficientCO2HighTemp(p, T, co2_density, xco2, yh2o);

  ABS_TEST(phiH2O, 0.156304067968, 1.0e-10);
  ABS_TEST(phiCO2, 0.641936764138, 1.0e-10);

  // Test that the same results are returned in fugacityCoefficientsHighTemp()
  Real phiCO2_2, phiH2O_2;
  _fp->fugacityCoefficientsHighTemp(p, T, co2_density, xco2, yh2o, phiCO2_2, phiH2O_2);

  ABS_TEST(phiH2O, phiH2O_2, 1.0e-12);
  ABS_TEST(phiCO2, phiCO2_2, 1.0e-12);
}

/*
 * Verify calculation of the H2O and CO2 activity coefficients and their derivative
 * wrt temperature
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficients)
{
  const Real xco2 = 0.01;
  Real T = 350.0;

  Real gammaH2O = _fp->activityCoefficientH2O(T, xco2);
  Real gammaCO2 = _fp->activityCoefficientCO2(T, xco2);

  ABS_TEST(gammaH2O, 1.0, 1.0e-10);
  ABS_TEST(gammaCO2, 1.0, 1.0e-10);

  T = 450.0;

  gammaH2O = _fp->activityCoefficientH2O(T, xco2);
  gammaCO2 = _fp->activityCoefficientCO2(T, xco2);

  ABS_TEST(gammaH2O, 1.00022113664, 1.0e-10);
  ABS_TEST(gammaCO2, 0.956736800255, 1.0e-10);
}

/*
 * Verify calculation of the Duan and Sun activity coefficient and its derivatives wrt
 * pressure, temperature and salt mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficientCO2Brine)
{
  const Real p = 10.0e6;
  Real T = 350.0;
  Real Xnacl = 0.1;
  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  // Low temperature regime
  Real gamma, dgamma_dp, dgamma_dT, dgamma_dX;
  _fp->activityCoefficient(p, T, Xnacl, gamma, dgamma_dp, dgamma_dT, dgamma_dX);
  ABS_TEST(gamma, 1.43276649338, 1.0e-8);

  Real gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dX;
  _fp->activityCoefficient(p + dp, T, Xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dX);

  Real dgamma_dp_fd = (gamma_2 - gamma) / dp;
  REL_TEST(dgamma_dp, dgamma_dp_fd, 1.0e-6);

  _fp->activityCoefficient(p, T + dT, Xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dX);

  Real dgamma_dT_fd = (gamma_2 - gamma) / dT;
  REL_TEST(dgamma_dT, dgamma_dT_fd, 1.0e-6);

  _fp->activityCoefficient(p, T, Xnacl + dx, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dX);

  Real dgamma_dX_fd = (gamma_2 - gamma) / dx;
  REL_TEST(dgamma_dX, dgamma_dX_fd, 1.0e-6);

  // High temperature regime
  T = 523.15;
  _fp->activityCoefficientHighTemp(T, Xnacl, gamma, dgamma_dT, dgamma_dX);
  ABS_TEST(gamma, 1.50047006243, 1.0e-8);

  _fp->activityCoefficientHighTemp(T + dT, Xnacl, gamma_2, dgamma_2_dT, dgamma_2_dX);
  dgamma_dT_fd = (gamma_2 - gamma) / dT;
  REL_TEST(dgamma_dT, dgamma_dT_fd, 1.0e-6);

  _fp->activityCoefficientHighTemp(T, Xnacl + dx, gamma_2, dgamma_2_dT, dgamma_2_dX);
  dgamma_dX_fd = (gamma_2 - gamma) / dx;
  REL_TEST(dgamma_dX, dgamma_dX_fd, 1.0e-6);

  // Check that both formulations return gamma = 1 for Xnacl = 0
  Xnacl = 0.0;
  T = 350.0;
  _fp->activityCoefficient(p, T, Xnacl, gamma, dgamma_dp, dgamma_dT, dgamma_dX);
  ABS_TEST(gamma, 1.0, 1.0e-12);

  T = 523.15;
  _fp->activityCoefficientHighTemp(T, Xnacl, gamma, dgamma_dT, dgamma_dX);
  ABS_TEST(gamma, 1.0, 1.0e-12);
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
  ABS_TEST(partial_density, 893.332, 1.0e-3);

  Real partial_density_2, dpartial_density_2_dT;
  _fp->partialDensityCO2(T + dT, partial_density_2, dpartial_density_2_dT);

  Real dpartial_density_dT_fd = (partial_density_2 - partial_density) / dT;
  REL_TEST(dpartial_density_dT, dpartial_density_dT_fd, 1.0e-6);
}

/*
 * Verify calculation of equilibrium mass fraction and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMassFraction)
{
  // Low temperature regime
  Real p = 1.0e6;
  Real T = 350.0;
  const Real Xnacl = 0.1;
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  Real X, dX_dp, dX_dT, dX_dX, Y, dY_dp, dY_dT, dY_dX;
  Real X1, dX1_dp, dX1_dT, dX1_dX, Y1, dY1_dp, dY1_dT, dY1_dX;
  Real X2, dX2_dp, dX2_dT, dX2_dX, Y2, dY2_dp, dY2_dT, dY2_dX;
  _fp->equilibriumMassFractions(p, T, Xnacl, X, dX_dp, dX_dT, dX_dX, Y, dY_dp, dY_dT, dY_dX);

  ABS_TEST(X, 0.00355727945939, 1.0e-10);
  ABS_TEST(Y, 0.0171978439739, 1.0e-10);

  // Derivative wrt pressure
  _fp->equilibriumMassFractions(
      p - dp, T, Xnacl, X1, dX1_dp, dX1_dT, dX1_dX, Y1, dY1_dp, dY1_dT, dY1_dX);
  _fp->equilibriumMassFractions(
      p + dp, T, Xnacl, X2, dX2_dp, dX2_dT, dX2_dX, Y2, dY2_dp, dY2_dT, dY2_dX);

  Real dX_dp_fd = (X2 - X1) / (2.0 * dp);
  Real dY_dp_fd = (Y2 - Y1) / (2.0 * dp);

  REL_TEST(dX_dp, dX_dp_fd, 1.0e-6);
  REL_TEST(dY_dp, dY_dp_fd, 1.0e-6);

  // Derivative wrt temperature
  _fp->equilibriumMassFractions(
      p, T - dT, Xnacl, X1, dX1_dp, dX1_dT, dX1_dX, Y1, dY1_dp, dY1_dT, dY1_dX);
  _fp->equilibriumMassFractions(
      p, T + dT, Xnacl, X2, dX2_dp, dX2_dT, dX2_dX, Y2, dY2_dp, dY2_dT, dY2_dX);

  Real dX_dT_fd = (X2 - X1) / (2.0 * dT);
  Real dY_dT_fd = (Y2 - Y1) / (2.0 * dT);

  REL_TEST(dX_dT, dX_dT_fd, 1.0e-5);
  REL_TEST(dY_dT, dY_dT_fd, 1.0e-5);

  // Derivative wrt salt mass fraction
  _fp->equilibriumMassFractions(
      p, T, Xnacl - dx, X1, dX1_dp, dX1_dT, dX1_dX, Y1, dY1_dp, dY1_dT, dY1_dX);
  _fp->equilibriumMassFractions(
      p, T, Xnacl + dx, X2, dX2_dp, dX2_dT, dX2_dX, Y2, dY2_dp, dY2_dT, dY2_dX);

  Real dX_dX_fd = (X2 - X1) / (2.0 * dx);
  Real dY_dX_fd = (Y2 - Y1) / (2.0 * dx);

  REL_TEST(dX_dX, dX_dX_fd, 1.0e-6);
  REL_TEST(dY_dX, dY_dX_fd, 1.0e-6);

  // High temperature regime
  p = 10.0e6;
  T = 525.15;

  Real x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX;
  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);

  _fp->equilibriumMassFractions(p, T, Xnacl, X, dX_dp, dX_dT, dX_dX, Y, dY_dp, dY_dT, dY_dX);

  ABS_TEST(X, 0.0162994976121, 1.0e-10);
  ABS_TEST(Y, 0.24947151051, 1.0e-10);
}

/*
 * Verify calculation of actual mass fraction and derivatives depending on value of
 * total mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, MassFraction)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real Xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  Real Z = 0.0001;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  Real Xco2 = fsp[0].mass_fraction[1];
  Real Yco2 = fsp[1].mass_fraction[1];
  Real Xh2o = fsp[0].mass_fraction[0];
  Real Yh2o = fsp[1].mass_fraction[0];
  Real Xnacl2 = fsp[0].mass_fraction[2];
  ABS_TEST(Xco2, Z, 1.0e-8);
  ABS_TEST(Yco2, 0.0, 1.0e-8);
  ABS_TEST(Xh2o, 1.0 - Z, 1.0e-8);
  ABS_TEST(Yh2o, 0.0, 1.0e-8);
  ABS_TEST(Xnacl2, Xnacl, 1.0e-8);

  // Verify derivatives
  Real dXco2_dp = fsp[0].dmass_fraction_dp[1];
  Real dXco2_dT = fsp[0].dmass_fraction_dT[1];
  Real dXco2_dX = fsp[0].dmass_fraction_dX[1];
  Real dXco2_dZ = fsp[0].dmass_fraction_dZ[1];
  Real dYco2_dp = fsp[1].dmass_fraction_dp[1];
  Real dYco2_dT = fsp[1].dmass_fraction_dT[1];
  Real dYco2_dX = fsp[1].dmass_fraction_dX[1];
  Real dYco2_dZ = fsp[1].dmass_fraction_dZ[1];
  Real dXnacl_dX = fsp[0].dmass_fraction_dX[2];

  ABS_TEST(dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dX, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dZ, 1.0, 1.0e-8);
  ABS_TEST(dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dX, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dZ, 0.0, 1.0e-8);
  ABS_TEST(dXnacl_dX, 1.0, 1.0e-8);

  // Gas region
  Z = 0.995;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  Real Ynacl = fsp[1].mass_fraction[2];
  ABS_TEST(Xco2, 0.0, 1.0e-8);
  ABS_TEST(Yco2, Z, 1.0e-8);
  ABS_TEST(Xh2o, 0.0, 1.0e-8);
  ABS_TEST(Yh2o, 1.0 - Z, 1.0e-8);
  ABS_TEST(Ynacl, 0.0, 1.0e-8);

  // Verify derivatives
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dX = fsp[0].dmass_fraction_dX[1];
  dXco2_dZ = fsp[0].dmass_fraction_dZ[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dX = fsp[1].dmass_fraction_dX[1];
  dYco2_dZ = fsp[1].dmass_fraction_dZ[1];
  Real dYnacl_dX = fsp[1].dmass_fraction_dX[2];
  ABS_TEST(dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dX, 0.0, 1.0e-8);
  ABS_TEST(dXco2_dZ, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dX, 0.0, 1.0e-8);
  ABS_TEST(dYnacl_dX, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dX, 0.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  // be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  Z = 0.45;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  Real Xco2_eq, dXco2_dp_eq, dXco2_dT_eq, dXco2_dX_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq,
      dYh2o_dX_eq;
  _fp->equilibriumMassFractions(p,
                                T,
                                Xnacl,
                                Xco2_eq,
                                dXco2_dp_eq,
                                dXco2_dT_eq,
                                dXco2_dX_eq,
                                Yh2o_eq,
                                dYh2o_dp_eq,
                                dYh2o_dT_eq,
                                dYh2o_dX_eq);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST(Xco2, Xco2_eq, 1.0e-8);
  ABS_TEST(Yco2, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST(Xh2o, 1.0 - Xco2_eq, 1.0e-8);
  ABS_TEST(Yh2o, Yh2o_eq, 1.0e-8);

  // Verify derivatives wrt p, T
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dX = fsp[0].dmass_fraction_dX[1];
  dXco2_dZ = fsp[0].dmass_fraction_dZ[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dX = fsp[1].dmass_fraction_dX[1];
  dYco2_dZ = fsp[1].dmass_fraction_dZ[1];

  ABS_TEST(dXco2_dp, dXco2_dp_eq, 1.0e-8);
  ABS_TEST(dXco2_dT, dXco2_dT_eq, 1.0e-8);
  ABS_TEST(dXco2_dX, dXco2_dX_eq, 1.0e-8);
  ABS_TEST(dXco2_dZ, 0.0, 1.0e-8);
  ABS_TEST(dYco2_dp, -dYh2o_dp_eq, 1.0e-8);
  ABS_TEST(dYco2_dT, -dYh2o_dT_eq, 1.0e-8);
  ABS_TEST(dYco2_dX, -dYh2o_dX_eq, 1.0e-8);
  ABS_TEST(dYco2_dZ, 0.0, 1.0e-8);

  // Use finite differences to verify derivative wrt Z is unaffected by Z
  const Real dZ = 1.0e-8;
  _fp->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  Real Xco21 = fsp[0].mass_fraction[1];
  Real Yco21 = fsp[1].mass_fraction[1];
  _fp->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  Real Xco22 = fsp[0].mass_fraction[1];
  Real Yco22 = fsp[1].mass_fraction[1];

  ABS_TEST(dXco2_dZ, (Xco21 - Xco22) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(dYco2_dZ, (Yco21 - Yco22) / (2.0 * dZ), 1.0e-8);
}

/*
 * Verify calculation of gas density, viscosity enthalpy, and derivatives. Note that as
 * these properties don't depend on mass fraction, only the gas region needs to be
 * tested (the calculations are identical in the two phase region)
 */
TEST_F(PorousFlowBrineCO2Test, gasProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real Xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Gas region
  Real Z = 0.995;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density, viscosity and enthalpy
  _fp->gasProperties(p, T, fsp);
  Real gas_density = fsp[1].density;
  Real gas_viscosity = fsp[1].viscosity;
  Real gas_enthalpy = fsp[1].enthalpy;

  Real density = _co2_fp->rho_from_p_T(p, T);
  Real viscosity = _co2_fp->mu_from_p_T(p, T);
  Real enthalpy = _co2_fp->h_from_p_T(p, T);

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
  _fp->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  ABS_TEST(ddensity_dZ, (rho1 - rho2) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(dviscosity_dZ, (mu1 - mu2) / (2.0 * dZ), 1.0e-8);
  ABS_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of liquid density, viscosity, enthalpy, and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, liquidProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real Xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  Real Z = 0.0001;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verify fluid density and viscosity
  _fp->liquidProperties(p, T, Xnacl, fsp);

  Real liquid_density = fsp[0].density;
  Real liquid_viscosity = fsp[0].viscosity;
  Real liquid_enthalpy = fsp[0].enthalpy;

  Real co2_partial_density, dco2_partial_density_dT;
  _fp->partialDensityCO2(T, co2_partial_density, dco2_partial_density_dT);
  Real brine_density = _brine_fp->rho(p, T, Xnacl);

  Real density = 1.0 / (Z / co2_partial_density + (1.0 - Z) / brine_density);

  Real viscosity = _brine_fp->mu(p, T, Xnacl);

  Real brine_enthalpy = _brine_fp->h(p, T, Xnacl);
  Real hdis, dhdis_dT;
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  Real co2_enthalpy = _co2_fp->h_from_p_T(p, T);
  Real enthalpy = (1.0 - Z) * brine_enthalpy + Z * (co2_enthalpy + hdis);

  ABS_TEST(liquid_density, density, 1.0e-12);
  ABS_TEST(liquid_viscosity, viscosity, 1.0e-12);
  ABS_TEST(liquid_enthalpy, enthalpy, 1.0e-12);

  // Verify fluid density and viscosity derivatives
  Real ddensity_dp = fsp[0].ddensity_dp;
  Real ddensity_dT = fsp[0].ddensity_dT;
  Real ddensity_dZ = fsp[0].ddensity_dZ;
  Real ddensity_dX = fsp[0].ddensity_dX;
  Real dviscosity_dp = fsp[0].dviscosity_dp;
  Real dviscosity_dT = fsp[0].dviscosity_dT;
  Real dviscosity_dZ = fsp[0].dviscosity_dZ;
  Real dviscosity_dX = fsp[0].dviscosity_dX;
  Real denthalpy_dp = fsp[0].denthalpy_dp;
  Real denthalpy_dT = fsp[0].denthalpy_dT;
  Real denthalpy_dZ = fsp[0].denthalpy_dZ;
  Real denthalpy_dX = fsp[0].denthalpy_dX;

  // Derivatives wrt pressure
  const Real dp = 1.0;
  _fp->liquidProperties(p + dp, T, Xnacl, fsp);
  Real rho1 = fsp[0].density;
  Real mu1 = fsp[0].viscosity;
  Real h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p - dp, T, Xnacl, fsp);
  Real rho2 = fsp[0].density;
  Real mu2 = fsp[0].viscosity;
  Real h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-4);

  // Derivatives wrt temperature
  const Real dT = 1.0e-4;
  _fp->liquidProperties(p, T + dT, Xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p, T - dT, Xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt Xnacl
  const Real dx = 1.0e-8;
  _fp->liquidProperties(p, T, Xnacl + dx, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p, T, Xnacl - dx, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dX, (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST(dviscosity_dX, (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST(denthalpy_dX, (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt Z
  const Real dZ = 1.0e-8;
  _fp->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dZ, (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(dviscosity_dZ, (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  REL_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);

  // Two-phase region
  Z = 0.045;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Verify fluid density and viscosity derivatives
  _fp->liquidProperties(p, T, Xnacl, fsp);

  ddensity_dp = fsp[0].ddensity_dp;
  ddensity_dT = fsp[0].ddensity_dT;
  ddensity_dX = fsp[0].ddensity_dX;
  ddensity_dZ = fsp[0].ddensity_dZ;
  dviscosity_dp = fsp[0].dviscosity_dp;
  dviscosity_dT = fsp[0].dviscosity_dT;
  dviscosity_dX = fsp[0].dviscosity_dX;
  dviscosity_dZ = fsp[0].dviscosity_dZ;
  denthalpy_dp = fsp[0].denthalpy_dp;
  denthalpy_dT = fsp[0].denthalpy_dT;
  denthalpy_dZ = fsp[0].denthalpy_dZ;
  denthalpy_dX = fsp[0].denthalpy_dX;

  // Derivatives wrt pressure
  _fp->massFractions(p + dp, T, Xnacl, Z, phase_state, fsp);
  _fp->liquidProperties(p + dp, T, Xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p - dp, T, Xnacl, Z, phase_state, fsp);
  _fp->liquidProperties(p - dp, T, Xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST(dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);
  REL_TEST(denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-4);

  // Derivatives wrt temperature
  _fp->massFractions(p, T + dT, Xnacl, Z, phase_state, fsp);
  _fp->liquidProperties(p, T + dT, Xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T - dT, Xnacl, Z, phase_state, fsp);
  _fp->liquidProperties(p, T - dT, Xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST(dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST(denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt Xnacl
  _fp->massFractions(p, T, Xnacl + dx, Z, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl + dx, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, Xnacl - dx, Z, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl - dx, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST(ddensity_dX, (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST(dviscosity_dX, (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST(denthalpy_dX, (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt Z
  _fp->massFractions(p, T, Xnacl, Z + dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, Xnacl, Z - dZ, phase_state, fsp);
  _fp->liquidProperties(p, T, Xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  ABS_TEST(ddensity_dZ, (rho1 - rho2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(dviscosity_dZ, (mu1 - mu2) / (2.0 * dZ), 1.0e-6);
  ABS_TEST(denthalpy_dZ, (h1 - h2) / (2.0 * dZ), 1.0e-6);
}

/*
 * Verify calculation of gas saturation and derivatives in the two-phase region
 */
TEST_F(PorousFlowBrineCO2Test, saturationTwoPhase)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real Xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // In the two-phase region, the mass fractions are the equilibrium values, so
  // a temporary value of Z can be used (as long as it corresponds to the two-phase
  // region)
  Real Z = 0.45;
  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Calculate Z that gives a saturation of 0.25
  Real gas_saturation = 0.25;
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - gas_saturation);
  // Calculate gas density and liquid density
  _fp->gasProperties(p, T, fsp);
  _fp->liquidProperties(liquid_pressure, T, Xnacl, fsp);

  // The mass fraction that corresponds to a gas_saturation = 0.25
  Z = (gas_saturation * fsp[1].density * fsp[1].mass_fraction[1] +
       (1.0 - gas_saturation) * fsp[0].density * fsp[0].mass_fraction[1]) /
      (gas_saturation * fsp[1].density + (1.0 - gas_saturation) * fsp[0].density);

  // Calculate the gas saturation and derivatives
  _fp->saturationTwoPhase(p, T, Xnacl, Z, fsp);

  ABS_TEST(fsp[1].saturation, gas_saturation, 1.0e-8);

  // Test the derivatives
  const Real dp = 1.0e-1;
  gas_saturation = fsp[1].saturation;
  Real dgas_saturation_dp = fsp[1].dsaturation_dp;
  Real dgas_saturation_dT = fsp[1].dsaturation_dT;
  Real dgas_saturation_dX = fsp[1].dsaturation_dX;
  Real dgas_saturation_dZ = fsp[1].dsaturation_dZ;

  // Derivative wrt pressure
  _fp->massFractions(p + dp, T, Xnacl, Z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  _fp->saturationTwoPhase(p + dp, T, Xnacl, Z, fsp);
  Real gsat1 = fsp[1].saturation;

  _fp->massFractions(p - dp, T, Xnacl, Z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  _fp->saturationTwoPhase(p - dp, T, Xnacl, Z, fsp);
  Real gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dp, (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt temperature
  const Real dT = 1.0e-4;
  _fp->massFractions(p, T + dT, Xnacl, Z, phase_state, fsp);
  _fp->gasProperties(p, T + dT, fsp);
  _fp->saturationTwoPhase(p, T + dT, Xnacl, Z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T - dT, Xnacl, Z, phase_state, fsp);
  _fp->gasProperties(p, T - dT, fsp);
  _fp->saturationTwoPhase(p, T - dT, Xnacl, Z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dT, (gsat1 - gsat2) / (2.0 * dT), 1.0e-6);

  // Derivative wrt Xnacl
  const Real dx = 1.0e-8;
  _fp->massFractions(p, T, Xnacl + dx, Z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, Xnacl + dx, Z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T, Xnacl - dx, Z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, Xnacl - dx, Z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dX, (gsat1 - gsat2) / (2.0 * dx), 1.0e-6);

  // Derivative wrt Z
  const Real dZ = 1.0e-8;

  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, Xnacl, Z + dZ, fsp);
  gsat1 = fsp[1].saturation;

  _fp->saturationTwoPhase(p, T, Xnacl, Z - dZ, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST(dgas_saturation_dZ, (gsat1 - gsat2) / (2.0 * dZ), 1.0e-6);
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

  Real Z = _fp->totalMassFraction(p, T, Xnacl, s, qp);

  // Test that the saturation calculated in this fluid state using Z is equal to s
  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  _fp->massFractions(p, T, Xnacl, Z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - s);
  _fp->liquidProperties(liquid_pressure, T, Xnacl, fsp);
  _fp->saturationTwoPhase(p, T, Xnacl, Z, fsp);
  ABS_TEST(fsp[1].saturation, s, 1.0e-8);
}

/*
 * Verify calculation of Henry constant with brine correction. Note: the values
 * calculated compare well by eye to the values presented in Figure 4 of
 * Battistelli et al, "A fluid property module for the TOUGH2 simulator for saline
 * brines with non-condensible gas"
 */
TEST_F(PorousFlowBrineCO2Test, henryConstant)
{
  Real T = 373.15;
  Real Xnacl = 0.1;

  Real Kh, dKh_dT, dKh_dX;
  _fp->henryConstant(T, Xnacl, Kh, dKh_dT, dKh_dX);
  REL_TEST(Kh, 7.46559e+08, 1.0e-3);

  T = 473.15;
  Xnacl = 0.2;

  _fp->henryConstant(T, Xnacl, Kh, dKh_dT, dKh_dX);
  REL_TEST(Kh, 1.66069e+09, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real Kh2, dKh2_dT, dKh2_dX;
  _fp->henryConstant(T + dT, Xnacl, Kh, dKh_dT, dKh_dX);
  _fp->henryConstant(T - dT, Xnacl, Kh2, dKh2_dT, dKh2_dX);

  REL_TEST(dKh_dT, (Kh - Kh2) / (2.0 * dT), 1.0e-5);

  // Test the derivative wrt Xnacl
  const Real dx = 1.0e-8;
  _fp->henryConstant(T, Xnacl + dx, Kh, dKh_dT, dKh_dX);
  _fp->henryConstant(T, Xnacl - dx, Kh2, dKh2_dT, dKh2_dX);

  REL_TEST(dKh_dX, (Kh - Kh2) / (2.0 * dx), 1.0e-5);
}

/*
 * Verify calculation of enthalpy of dissolution. Note: the values calculated compare
 * well by eye to the values presented in Figure 4 of Battistelli et al, "A fluid property
 * module for the TOUGH2 simulator for saline brines with non-condensible gas"
 */
TEST_F(PorousFlowBrineCO2Test, enthalpyOfDissolutionGas)
{
  // T = 50C
  Real T = 323.15;
  Real Xnacl = 0.1;

  // Enthalpy of dissolution of CO2 in brine
  Real hdis, dhdis_dT, dhdis_dX;
  _fp->enthalpyOfDissolutionGas(T, Xnacl, hdis, dhdis_dT, dhdis_dX);
  REL_TEST(hdis, -3.20130e5, 1.0e-3);

  // T = 350C
  T = 623.15;

  // Enthalpy of dissolution of CO2 in brine
  _fp->enthalpyOfDissolutionGas(T, Xnacl, hdis, dhdis_dT, dhdis_dX);
  REL_TEST(hdis, 9.83813e+05, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1, dhdis1_dT, dhdis1_dX, hdis2, dhdis2_dT, dhdis2_dX;
  _fp->enthalpyOfDissolutionGas(T + dT, Xnacl, hdis1, dhdis1_dT, dhdis1_dX);
  _fp->enthalpyOfDissolutionGas(T - dT, Xnacl, hdis2, dhdis2_dT, dhdis2_dX);

  REL_TEST(dhdis_dT, (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);

  // Test the derivative wrt salt mass fraction
  const Real dx = 1.0e-8;
  _fp->enthalpyOfDissolutionGas(T, Xnacl + dx, hdis1, dhdis1_dT, dhdis1_dX);
  _fp->enthalpyOfDissolutionGas(T, Xnacl - dx, hdis2, dhdis2_dT, dhdis2_dX);

  REL_TEST(dhdis_dX, (hdis1 - hdis2) / (2.0 * dx), 1.0e-5);
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
  Real T = 323.15;

  // Enthalpy of dissolution of CO2 in water
  Real hdis, dhdis_dT;
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  REL_TEST(hdis, -3.38185e5, 1.0e-3);

  // T = 350C
  T = 623.15;

  // Enthalpy of dissolution of CO2 in water
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  REL_TEST(hdis, 5.78787e5, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1, dhdis1_dT, hdis2, dhdis2_dT;
  _fp->enthalpyOfDissolution(T + dT, hdis1, dhdis1_dT);
  _fp->enthalpyOfDissolution(T - dT, hdis2, dhdis2_dT);

  REL_TEST(dhdis_dT, (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);
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
  _fp->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.161429471353, 1.0e-10);
  ABS_TEST(xco2, 0.0236967010229, 1.0e-10);

  // Mass fraction equivalent to molality of 2
  p = 30.0e6;
  T = 423.15;
  Xnacl = 0.105;

  _fp->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.0468195468869, 1.0e-10);
  ABS_TEST(xco2, 0.023664581533, 1.0e-10);

  // Mass fraction equivalent to molality of 4
  T = 523.15;
  Xnacl = 0.189;

  co2_density = _co2_fp->rho_from_p_T(p, T);
  _fp->solveEquilibriumMoleFractionHighTemp(p, T, Xnacl, co2_density, xco2, yh2o);
  ABS_TEST(yh2o, 0.253292227782, 1.0e-10);
  ABS_TEST(xco2, 0.016834474171, 1.0e-10);
}

/**
 * Test calculation of equilibrium mole fractions over entire temperature range
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMoleFractions)
{
  // Test pure water (Xnacl = 0)
  // Low temperature regime
  Real p = 20.0e6;
  Real T = 323.15;
  Real Xnacl = 0.0;

  Real x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX;
  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.00696382327418, 1.0e-8);
  ABS_TEST(x, 0.0236534984353, 1.0e-8);

  // Intermediate temperature regime
  T = 373.15;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.0194363435584, 1.0e-8);
  ABS_TEST(x, 0.0201949380648, 1.0e-8);

  // High temperature regime
  p = 30.0e6;
  T = 523.15;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.286116587269, 1.0e-8);
  ABS_TEST(x, 0.0409622847096, 1.0e-8);

  // High temperature regime with low pressure
  p = 1.0e6;
  T = 523.15;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 1.0, 1.0e-10);
  ABS_TEST(x, 0.0, 1.0e-10);

  // Test brine (Xnacl = 0.1)
  // Low temperature regime
  p = 20.0e6;
  T = 323.15;
  Xnacl = 0.1;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.00657324509341, 1.0e-8);
  ABS_TEST(x, 0.0152851189462, 1.0e-8);

  // Intermediate temperature regime
  T = 373.15;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.0183104919388, 1.0e-8);
  ABS_TEST(x, 0.0132647919449, 1.0e-8);

  // High temperature regime
  p = 30.0e6;
  T = 523.15;

  _fp->equilibriumMoleFractions(p, T, Xnacl, x, dx_dp, dx_dT, dx_dX, y, dy_dp, dy_dT, dy_dX);
  ABS_TEST(y, 0.270258370983, 1.0e-8);
  ABS_TEST(x, 0.0246589523314, 1.0e-8);
}

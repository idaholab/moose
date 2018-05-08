//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrineCO2Test.h"
#include "Utils.h"

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

  ABS_TEST("K0H2O", K0H2O, 0.412597711705, 1.0e-10);
  ABS_TEST("K0CO2", K0CO2, 74.0435888596, 1.0e-10);

  Real K0H2O_2, dK0H2O_2_dT, K0CO2_2, dK0CO2_2_dT;
  _fp->equilibriumConstantH2O(T + dT, K0H2O_2, dK0H2O_2_dT);
  _fp->equilibriumConstantCO2(T + dT, K0CO2_2, dK0CO2_2_dT);

  Real dK0H2O_dT_fd = (K0H2O_2 - K0H2O) / dT;
  Real dK0CO2_dT_fd = (K0CO2_2 - K0CO2) / dT;
  REL_TEST("dK0H2O_dT", dK0H2O_dT, dK0H2O_dT_fd, 1.0e-6);
  REL_TEST("dK0CO2_dT", dK0CO2_dT, dK0CO2_dT_fd, 1.0e-6);

  // Test the high temperature formulation
  T = 450.0;

  _fp->equilibriumConstantH2O(T, K0H2O, dK0H2O_dT);
  _fp->equilibriumConstantCO2(T, K0CO2, dK0CO2_dT);

  ABS_TEST("K0H2O", K0H2O, 8.75944517916, 1.0e-10);
  ABS_TEST("K0CO2", K0CO2, 105.013867434, 1.0e-10);

  _fp->equilibriumConstantH2O(T + dT, K0H2O_2, dK0H2O_2_dT);
  _fp->equilibriumConstantCO2(T + dT, K0CO2_2, dK0CO2_2_dT);

  dK0H2O_dT_fd = (K0H2O_2 - K0H2O) / dT;
  dK0CO2_dT_fd = (K0CO2_2 - K0CO2) / dT;
  REL_TEST("dK0H2O_dT", dK0H2O_dT, dK0H2O_dT_fd, 1.0e-6);
  REL_TEST("dK0CO2_dT", dK0CO2_dT, dK0CO2_dT_fd, 1.0e-5);
}

/*
 * Verify calculation of the fugacity coefficients
 */
TEST_F(PorousFlowBrineCO2Test, fugacityCoefficients)
{
  // Test the low temperature formulation
  Real p = 40.0e6;
  Real T = 350.0;

  Real phiH2O, dphiH2O_dp, dphiH2O_dT;
  Real phiCO2, dphiCO2_dp, dphiCO2_dT;
  _fp->fugacityCoefficientsLowTemp(
      p, T, phiCO2, dphiCO2_dp, dphiCO2_dT, phiH2O, dphiH2O_dp, dphiH2O_dT);

  ABS_TEST("phiCO2", phiCO2, 0.401938, 1.0e-6);
  ABS_TEST("phiH2O", phiH2O, 0.0898968, 1.0e-6);

  // Make sure that the method fugacityCoefficients() correctly returns the same values
  // when infinite dilution is assumed
  Real yh2o = 0.0, xco2 = 0.0;

  Real phiCO2_2, dphiCO2_2_dp, dphiCO2_2_dT, phiH2O_2, dphiH2O_2_dp, dphiH2O_2_dT;
  _fp->fugacityCoefficients(
      p, T, xco2, yh2o, phiCO2_2, dphiCO2_2_dp, dphiCO2_2_dT, phiH2O_2, dphiH2O_2_dp, dphiH2O_2_dT);

  ABS_TEST("phiH2O", phiH2O, phiH2O_2, 1.0e-12);
  ABS_TEST("phiCO2", phiCO2, phiCO2_2, 1.0e-12);
  ABS_TEST("dphiH2O_dp", dphiH2O_dp, dphiH2O_2_dp, 1.0e-12);
  ABS_TEST("dphiH2O_dT", dphiH2O_dT, dphiH2O_2_dT, 1.0e-12);
  ABS_TEST("dphiCO2_dp", dphiCO2_dp, dphiCO2_2_dp, 1.0e-12);
  ABS_TEST("dphiCO2_dT", dphiCO2_dT, dphiCO2_2_dT, 1.0e-12);

  // Test the high temperature formulation
  T = 423.15;

  _fp->solveEquilibriumHighTemp(p, T, xco2, yh2o);
  phiH2O = _fp->fugacityCoefficientH2OHighTemp(p, T, xco2, yh2o);
  phiCO2 = _fp->fugacityCoefficientCO2HighTemp(p, T, xco2, yh2o);

  ABS_TEST("phiH2O", phiH2O, 0.156304067968, 1.0e-10);
  ABS_TEST("phiCO2", phiCO2, 0.641936764138, 1.0e-10);

  // Test that the same results are returned in fugacityCoefficientsHighTemp()
  _fp->fugacityCoefficientsHighTemp(
      p, T, xco2, yh2o, phiCO2_2, dphiCO2_dp, dphiCO2_dT, phiH2O_2, dphiH2O_dp, dphiH2O_dT);

  ABS_TEST("phiH2O", phiH2O, phiH2O_2, 1.0e-12);
  ABS_TEST("phiCO2", phiCO2, phiCO2_2, 1.0e-12);

  // Test that the same results are returned in fugacityCoefficients()
  _fp->fugacityCoefficients(
      p, T, xco2, yh2o, phiCO2_2, dphiCO2_dp, dphiCO2_dT, phiH2O_2, dphiH2O_dp, dphiH2O_dT);

  ABS_TEST("phiH2O", phiH2O, phiH2O_2, 1.0e-12);
  ABS_TEST("phiCO2", phiCO2, phiCO2_2, 1.0e-12);
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

  ABS_TEST("gammaH2O", gammaH2O, 1.0, 1.0e-10);
  ABS_TEST("gammaCO2", gammaCO2, 1.0, 1.0e-10);

  T = 450.0;

  gammaH2O = _fp->activityCoefficientH2O(T, xco2);
  gammaCO2 = _fp->activityCoefficientCO2(T, xco2);

  ABS_TEST("gammaH2O", gammaH2O, 1.00022113664, 1.0e-10);
  ABS_TEST("gammaCO2", gammaCO2, 0.956736800255, 1.0e-10);
}

/*
 * Verify calculation of the salting out activity coefficient and its derivatives wrt
 * pressure, temperature and salt mass fraction
 */
TEST_F(PorousFlowBrineCO2Test, activityCoefficientSaltingOut)
{
  const Real p = 10.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;
  const Real dp = 1.0e-1;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  Real gamma, dgamma_dp, dgamma_dT, dgamma_dx;
  _fp->activityCoefficient(p, T, xnacl, gamma, dgamma_dp, dgamma_dT, dgamma_dx);
  ABS_TEST("gamma", gamma, 1.43, 1.0e-2);

  Real gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dx;
  _fp->activityCoefficient(p + dp, T, xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dx);

  Real dgamma_dp_fd = (gamma_2 - gamma) / dp;
  REL_TEST("dgamma_dp", dgamma_dp, dgamma_dp_fd, 1.0e-6);

  _fp->activityCoefficient(p, T + dT, xnacl, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dx);

  Real dgamma_dT_fd = (gamma_2 - gamma) / dT;
  REL_TEST("dgamma_dT", dgamma_dT, dgamma_dT_fd, 1.0e-6);

  _fp->activityCoefficient(p, T, xnacl + dx, gamma_2, dgamma_2_dp, dgamma_2_dT, dgamma_2_dx);

  Real dgamma_dx_fd = (gamma_2 - gamma) / dx;
  REL_TEST("dgamma_dx", dgamma_dx, dgamma_dx_fd, 1.0e-6);
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
  // Low temperature regime
  Real p = 1.0e6;
  Real T = 350.0;
  const Real xnacl = 0.1;
  const Real dp = 1.0e-2;
  const Real dT = 1.0e-6;
  const Real dx = 1.0e-8;

  Real X, dX_dp, dX_dT, dX_dx, Y, dY_dp, dY_dT, dY_dx;
  Real X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx;
  Real X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx;
  _fp->equilibriumMassFractions(p, T, xnacl, X, dX_dp, dX_dT, dX_dx, Y, dY_dp, dY_dT, dY_dx);

  ABS_TEST("Xco2", X, 0.00355728, 1.0e-6);
  ABS_TEST("Yh2o", Y, 0.0171978, 1.0e-6);

  // Derivative wrt pressure
  _fp->equilibriumMassFractions(
      p - dp, T, xnacl, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p + dp, T, xnacl, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  Real dX_dp_fd = (X2 - X1) / (2.0 * dp);
  Real dY_dp_fd = (Y2 - Y1) / (2.0 * dp);

  REL_TEST("dXco2_dp", dX_dp, dX_dp_fd, 1.0e-6);
  REL_TEST("dYh2o_dp", dY_dp, dY_dp_fd, 1.0e-6);

  // Derivative wrt temperature
  _fp->equilibriumMassFractions(
      p, T - dT, xnacl, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p, T + dT, xnacl, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  Real dX_dT_fd = (X2 - X1) / (2.0 * dT);
  Real dY_dT_fd = (Y2 - Y1) / (2.0 * dT);

  REL_TEST("dXco2_dT", dX_dT, dX_dT_fd, 1.0e-5);
  REL_TEST("dYh2o_dT", dY_dT, dY_dT_fd, 1.0e-5);

  // Derivative wrt salt mass fraction
  _fp->equilibriumMassFractions(
      p, T, xnacl - dx, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p, T, xnacl + dx, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  Real dX_dx_fd = (X2 - X1) / (2.0 * dx);
  Real dY_dx_fd = (Y2 - Y1) / (2.0 * dx);

  REL_TEST("dXco2_dx", dX_dx, dX_dx_fd, 1.0e-6);
  REL_TEST("dYh2o_dx", dY_dx, dY_dx_fd, 1.0e-6);

  // High temperature regime
  p = 10.0e6;
  T = 525.15;

  Real x, y;
  _fp->equilibriumMoleFractions(p, T, x, y);

  _fp->equilibriumMassFractions(p, T, xnacl, X, dX_dp, dX_dT, dX_dx, Y, dY_dp, dY_dT, dY_dx);

  ABS_TEST("Xco2", X, 0.0139596379913, 1.0e-10);
  ABS_TEST("Yh2o", Y, 0.250175712551, 1.0e-10);

  // Derivative wrt pressure
  _fp->equilibriumMassFractions(
      p - dp, T, xnacl, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p + dp, T, xnacl, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  dX_dp_fd = (X2 - X1) / (2.0 * dp);
  dY_dp_fd = (Y2 - Y1) / (2.0 * dp);

  REL_TEST("dXco2_dp", dX_dp, dX_dp_fd, 1.0e-1);
  REL_TEST("dYh2o_dp", dY_dp, dY_dp_fd, 1.0e-1);

  // Derivative wrt temperature
  _fp->equilibriumMassFractions(
      p, T - dT, xnacl, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p, T + dT, xnacl, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  dX_dT_fd = (X2 - X1) / (2.0 * dT);
  dY_dT_fd = (Y2 - Y1) / (2.0 * dT);

  REL_TEST("dXco2_dT", dX_dT, dX_dT_fd, 1.0e-1);
  REL_TEST("dYh2o_dT", dY_dT, dY_dT_fd, 1.0e-1);

  // Derivative wrt salt mass fraction
  _fp->equilibriumMassFractions(
      p, T, xnacl - dx, X1, dX1_dp, dX1_dT, dX1_dx, Y1, dY1_dp, dY1_dT, dY1_dx);
  _fp->equilibriumMassFractions(
      p, T, xnacl + dx, X2, dX2_dp, dX2_dT, dX2_dx, Y2, dY2_dp, dY2_dT, dY2_dx);

  dX_dx_fd = (X2 - X1) / (2.0 * dx);
  dY_dx_fd = (Y2 - Y1) / (2.0 * dx);

  REL_TEST("dXco2_dx", dX_dx, dX_dx_fd, 1.0e-6);
  REL_TEST("dYh2o_dx", dY_dx, dY_dx_fd, 1.0e-6);
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
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  Real z = 0.0001;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verfify mass fraction values
  Real Xco2 = fsp[0].mass_fraction[1];
  Real Yco2 = fsp[1].mass_fraction[1];
  Real Xh2o = fsp[0].mass_fraction[0];
  Real Yh2o = fsp[1].mass_fraction[0];
  Real Xnacl = fsp[0].mass_fraction[2];
  ABS_TEST("Xco2", Xco2, z, 1.0e-8);
  ABS_TEST("Yco2", Yco2, 0.0, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - z, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 0.0, 1.0e-8);
  ABS_TEST("Xnacl", Xnacl, xnacl, 1.0e-8);

  // Verify derivatives
  Real dXco2_dp = fsp[0].dmass_fraction_dp[1];
  Real dXco2_dT = fsp[0].dmass_fraction_dT[1];
  Real dXco2_dx = fsp[0].dmass_fraction_dx[1];
  Real dXco2_dz = fsp[0].dmass_fraction_dz[1];
  Real dYco2_dp = fsp[1].dmass_fraction_dp[1];
  Real dYco2_dT = fsp[1].dmass_fraction_dT[1];
  Real dYco2_dx = fsp[1].dmass_fraction_dx[1];
  Real dYco2_dz = fsp[1].dmass_fraction_dz[1];
  Real dXnacl_dx = fsp[0].dmass_fraction_dx[2];

  ABS_TEST("dXco2_dp", dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dx", dXco2_dx, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 1.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dx", dYco2_dx, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dz", dYco2_dz, 0.0, 1.0e-8);
  ABS_TEST("dXnacl_dx", dXnacl_dx, 1.0, 1.0e-8);

  // Gas region
  z = 0.995;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  Real Ynacl = fsp[1].mass_fraction[2];
  ABS_TEST("Xco2", Xco2, 0.0, 1.0e-8);
  ABS_TEST("Yco2", Yco2, z, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 0.0, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, 1.0 - z, 1.0e-8);
  ABS_TEST("Ynacl", Ynacl, 0.0, 1.0e-8);

  // Verify derivatives
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dx = fsp[0].dmass_fraction_dx[1];
  dXco2_dz = fsp[0].dmass_fraction_dz[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dx = fsp[1].dmass_fraction_dx[1];
  dYco2_dz = fsp[1].dmass_fraction_dz[1];
  Real dYnacl_dx = fsp[1].dmass_fraction_dx[2];
  ABS_TEST("dXco2_dp", dXco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dx", dXco2_dx, 0.0, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dx", dYco2_dx, 0.0, 1.0e-8);
  ABS_TEST("dYnacl_dz", dYnacl_dx, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dx", dYco2_dx, 0.0, 1.0e-8);

  // Two phase region. In this region, the mass fractions and derivatives can
  // be verified using the equilibrium mass fraction derivatives that have
  // been verified above
  z = 0.45;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Equilibrium mass fractions and derivatives
  Real Xco2_eq, dXco2_dp_eq, dXco2_dT_eq, dXco2_dx_eq, Yh2o_eq, dYh2o_dp_eq, dYh2o_dT_eq,
      dYh2o_dx_eq;
  _fp->equilibriumMassFractions(p,
                                T,
                                xnacl,
                                Xco2_eq,
                                dXco2_dp_eq,
                                dXco2_dT_eq,
                                dXco2_dx_eq,
                                Yh2o_eq,
                                dYh2o_dp_eq,
                                dYh2o_dT_eq,
                                dYh2o_dx_eq);

  // Verfify mass fraction values
  Xco2 = fsp[0].mass_fraction[1];
  Yco2 = fsp[1].mass_fraction[1];
  Xh2o = fsp[0].mass_fraction[0];
  Yh2o = fsp[1].mass_fraction[0];
  ABS_TEST("Xco2", Xco2, Xco2_eq, 1.0e-8);
  ABS_TEST("Yco2", Yco2, 1.0 - Yh2o_eq, 1.0e-8);
  ABS_TEST("Xh2o", Xh2o, 1.0 - Xco2_eq, 1.0e-8);
  ABS_TEST("Yh2o", Yh2o, Yh2o_eq, 1.0e-8);

  // Verify derivatives wrt p, T
  dXco2_dp = fsp[0].dmass_fraction_dp[1];
  dXco2_dT = fsp[0].dmass_fraction_dT[1];
  dXco2_dx = fsp[0].dmass_fraction_dx[1];
  dXco2_dz = fsp[0].dmass_fraction_dz[1];
  dYco2_dp = fsp[1].dmass_fraction_dp[1];
  dYco2_dT = fsp[1].dmass_fraction_dT[1];
  dYco2_dx = fsp[1].dmass_fraction_dx[1];
  dYco2_dz = fsp[1].dmass_fraction_dz[1];

  ABS_TEST("dXco2_dp", dXco2_dp, dXco2_dp_eq, 1.0e-8);
  ABS_TEST("dXco2_dT", dXco2_dT, dXco2_dT_eq, 1.0e-8);
  ABS_TEST("dXco2_dx", dXco2_dx, dXco2_dx_eq, 1.0e-8);
  ABS_TEST("dXco2_dz", dXco2_dz, 0.0, 1.0e-8);
  ABS_TEST("dYco2_dp", dYco2_dp, -dYh2o_dp_eq, 1.0e-8);
  ABS_TEST("dYco2_dT", dYco2_dT, -dYh2o_dT_eq, 1.0e-8);
  ABS_TEST("dYco2_dx", dYco2_dx, -dYh2o_dx_eq, 1.0e-8);
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
 * Verify calculation of gas density, viscosity enthalpy, and derivatives. Note that as
 * these properties don't depend on mass fraction, only the gas region needs to be
 * tested (the calculations are identical in the two phase region)
 */
TEST_F(PorousFlowBrineCO2Test, gasProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Gas region
  Real z = 0.995;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  // Verify fluid density, viscosity and enthalpy
  _fp->gasProperties(p, T, fsp);
  Real gas_density = fsp[1].density;
  Real gas_viscosity = fsp[1].viscosity;
  Real gas_enthalpy = fsp[1].enthalpy;

  Real density = _co2_fp->rho(p, T);
  Real viscosity = _co2_fp->mu(p, T);
  Real enthalpy = _co2_fp->h(p, T);

  ABS_TEST("gas density", gas_density, density, 1.0e-8);
  ABS_TEST("gas viscosity", gas_viscosity, viscosity, 1.0e-8);
  ABS_TEST("gas enthalpy", gas_enthalpy, enthalpy, 1.0e-8);

  // Verify derivatives
  Real ddensity_dp = fsp[1].ddensity_dp;
  Real ddensity_dT = fsp[1].ddensity_dT;
  Real ddensity_dz = fsp[1].ddensity_dz;
  Real dviscosity_dp = fsp[1].dviscosity_dp;
  Real dviscosity_dT = fsp[1].dviscosity_dT;
  Real dviscosity_dz = fsp[1].dviscosity_dz;
  Real denthalpy_dp = fsp[1].denthalpy_dp;
  Real denthalpy_dT = fsp[1].denthalpy_dT;
  Real denthalpy_dz = fsp[1].denthalpy_dz;

  const Real dp = 1.0e-1;
  _fp->gasProperties(p + dp, T, fsp);
  Real rho1 = fsp[1].density;
  Real mu1 = fsp[1].viscosity;
  Real h1 = fsp[1].enthalpy;

  _fp->gasProperties(p - dp, T, fsp);
  Real rho2 = fsp[1].density;
  Real mu2 = fsp[1].viscosity;
  Real h2 = fsp[1].enthalpy;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-6);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-6);
  REL_TEST("denthalpy_dp", denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-6);

  const Real dT = 1.0e-3;
  _fp->gasProperties(p, T + dT, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->gasProperties(p, T - dT, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST("denthalpy_dT", denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Note: mass fraction changes with z
  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho1 = fsp[1].density;
  mu1 = fsp[1].viscosity;
  h1 = fsp[1].enthalpy;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  rho2 = fsp[1].density;
  mu2 = fsp[1].viscosity;
  h2 = fsp[1].enthalpy;

  ABS_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-8);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-8);
  ABS_TEST("denthalpy_dz", denthalpy_dz, (h1 - h2) / (2.0 * dz), 1.0e-6);
}

/*
 * Verify calculation of liquid density, viscosity, enthalpy, and derivatives
 */
TEST_F(PorousFlowBrineCO2Test, liquidProperties)
{
  const Real p = 1.0e6;
  const Real T = 350.0;
  const Real xnacl = 0.1;

  FluidStatePhaseEnum phase_state;
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  // Liquid region
  Real z = 0.0001;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  // Verify fluid density and viscosity
  _fp->liquidProperties(p, T, xnacl, fsp);

  Real liquid_density = fsp[0].density;
  Real liquid_viscosity = fsp[0].viscosity;
  Real liquid_enthalpy = fsp[0].enthalpy;

  Real co2_partial_density, dco2_partial_density_dT;
  _fp->partialDensityCO2(T, co2_partial_density, dco2_partial_density_dT);
  Real brine_density = _brine_fp->rho(p, T, xnacl);

  Real density = 1.0 / (z / co2_partial_density + (1.0 - z) / brine_density);

  Real viscosity = _brine_fp->mu(p, T, xnacl);

  Real brine_enthalpy = _brine_fp->h(p, T, xnacl);
  Real hdis, dhdis_dT;
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  Real co2_enthalpy = _co2_fp->h(p, T);
  Real enthalpy = (1.0 - z) * brine_enthalpy + z * (co2_enthalpy + hdis);

  ABS_TEST("liquid density", liquid_density, density, 1.0e-12);
  ABS_TEST("liquid viscosity", liquid_viscosity, viscosity, 1.0e-12);
  ABS_TEST("liquid enthalpy", liquid_enthalpy, enthalpy, 1.0e-12);

  // Verify fluid density and viscosity derivatives
  Real ddensity_dp = fsp[0].ddensity_dp;
  Real ddensity_dT = fsp[0].ddensity_dT;
  Real ddensity_dz = fsp[0].ddensity_dz;
  Real ddensity_dx = fsp[0].ddensity_dx;
  Real dviscosity_dp = fsp[0].dviscosity_dp;
  Real dviscosity_dT = fsp[0].dviscosity_dT;
  Real dviscosity_dz = fsp[0].dviscosity_dz;
  Real dviscosity_dx = fsp[0].dviscosity_dx;
  Real denthalpy_dp = fsp[0].denthalpy_dp;
  Real denthalpy_dT = fsp[0].denthalpy_dT;
  Real denthalpy_dz = fsp[0].denthalpy_dz;
  Real denthalpy_dx = fsp[0].denthalpy_dx;

  // Derivatives wrt pressure
  const Real dp = 1.0;
  _fp->liquidProperties(p + dp, T, xnacl, fsp);
  Real rho1 = fsp[0].density;
  Real mu1 = fsp[0].viscosity;
  Real h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p - dp, T, xnacl, fsp);
  Real rho2 = fsp[0].density;
  Real mu2 = fsp[0].viscosity;
  Real h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);
  REL_TEST("denthalpy_dp", denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-4);

  // Derivatives wrt temperature
  const Real dT = 1.0e-4;
  _fp->liquidProperties(p, T + dT, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p, T - dT, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST("denthalpy_dT", denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt xnacl
  const Real dx = 1.0e-8;
  _fp->liquidProperties(p, T, xnacl + dx, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->liquidProperties(p, T, xnacl - dx, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dx", ddensity_dx, (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST("dviscosity_dx", dviscosity_dx, (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST("denthalpy_dx", denthalpy_dx, (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt z
  const Real dz = 1.0e-8;
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-6);
  REL_TEST("denthalpy_dz", denthalpy_dz, (h1 - h2) / (2.0 * dz), 1.0e-6);

  // Two-phase region
  z = 0.045;
  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  // Verify fluid density and viscosity derivatives
  _fp->liquidProperties(p, T, xnacl, fsp);

  ddensity_dp = fsp[0].ddensity_dp;
  ddensity_dT = fsp[0].ddensity_dT;
  ddensity_dx = fsp[0].ddensity_dx;
  ddensity_dz = fsp[0].ddensity_dz;
  dviscosity_dp = fsp[0].dviscosity_dp;
  dviscosity_dT = fsp[0].dviscosity_dT;
  dviscosity_dx = fsp[0].dviscosity_dx;
  dviscosity_dz = fsp[0].dviscosity_dz;
  denthalpy_dp = fsp[0].denthalpy_dp;
  denthalpy_dT = fsp[0].denthalpy_dT;
  denthalpy_dz = fsp[0].denthalpy_dz;
  denthalpy_dx = fsp[0].denthalpy_dx;

  // Derivatives wrt pressure
  _fp->massFractions(p + dp, T, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p + dp, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p - dp, T, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p - dp, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dp", ddensity_dp, (rho1 - rho2) / (2.0 * dp), 1.0e-4);
  REL_TEST("dviscosity_dp", dviscosity_dp, (mu1 - mu2) / (2.0 * dp), 1.0e-5);
  REL_TEST("denthalpy_dp", denthalpy_dp, (h1 - h2) / (2.0 * dp), 1.0e-4);

  // Derivatives wrt temperature
  _fp->massFractions(p, T + dT, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p, T + dT, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T - dT, xnacl, z, phase_state, fsp);
  _fp->liquidProperties(p, T - dT, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dT", ddensity_dT, (rho1 - rho2) / (2.0 * dT), 1.0e-6);
  REL_TEST("dviscosity_dT", dviscosity_dT, (mu1 - mu2) / (2.0 * dT), 1.0e-6);
  REL_TEST("denthalpy_dT", denthalpy_dT, (h1 - h2) / (2.0 * dT), 1.0e-6);

  // Derivatives wrt xnacl
  _fp->massFractions(p, T, xnacl + dx, z, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl + dx, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, xnacl - dx, z, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl - dx, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  REL_TEST("ddensity_dx", ddensity_dx, (rho1 - rho2) / (2.0 * dx), 1.0e-6);
  REL_TEST("dviscosity_dx", dviscosity_dx, (mu1 - mu2) / (2.0 * dx), 1.0e-6);
  REL_TEST("denthalpy_dx", denthalpy_dx, (h1 - h2) / (2.0 * dx), 1.0e-6);

  // Derivatives wrt z
  _fp->massFractions(p, T, xnacl, z + dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho1 = fsp[0].density;
  mu1 = fsp[0].viscosity;
  h1 = fsp[0].enthalpy;

  _fp->massFractions(p, T, xnacl, z - dz, phase_state, fsp);
  _fp->liquidProperties(p, T, xnacl, fsp);
  rho2 = fsp[0].density;
  mu2 = fsp[0].viscosity;
  h2 = fsp[0].enthalpy;

  ABS_TEST("ddensity_dz", ddensity_dz, (rho1 - rho2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("dviscosity_dz", dviscosity_dz, (mu1 - mu2) / (2.0 * dz), 1.0e-6);
  ABS_TEST("denthalpy_dz", denthalpy_dz, (h1 - h2) / (2.0 * dz), 1.0e-6);
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
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

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
  Real dgas_saturation_dx = fsp[1].dsaturation_dx;
  Real dgas_saturation_dz = fsp[1].dsaturation_dz;

  // Derivative wrt pressure
  _fp->massFractions(p + dp, T, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p + dp, T, fsp);
  _fp->saturationTwoPhase(p + dp, T, xnacl, z, fsp);
  Real gsat1 = fsp[1].saturation;

  _fp->massFractions(p - dp, T, xnacl, z, phase_state, fsp);
  _fp->gasProperties(p - dp, T, fsp);
  _fp->saturationTwoPhase(p - dp, T, xnacl, z, fsp);
  Real gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dp", dgas_saturation_dp, (gsat1 - gsat2) / (2.0 * dp), 1.0e-6);

  // Derivative wrt temperature
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

  // Derivative wrt xnacl
  const Real dx = 1.0e-8;
  _fp->massFractions(p, T, xnacl + dx, z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, xnacl + dx, z, fsp);
  gsat1 = fsp[1].saturation;

  _fp->massFractions(p, T, xnacl - dx, z, phase_state, fsp);
  _fp->gasProperties(p, T, fsp);
  _fp->saturationTwoPhase(p, T, xnacl - dx, z, fsp);
  gsat2 = fsp[1].saturation;

  REL_TEST("dgas_saturation_dx", dgas_saturation_dx, (gsat1 - gsat2) / (2.0 * dx), 1.0e-6);

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
  const unsigned int np = _fp->numPhases();
  const unsigned int nc = _fp->numComponents();
  std::vector<FluidStateProperties> fsp(np, FluidStateProperties(nc));

  _fp->massFractions(p, T, xnacl, z, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  _fp->gasProperties(p, T, fsp);
  Real liquid_pressure = p + _pc->capillaryPressure(1.0 - s);
  _fp->liquidProperties(liquid_pressure, T, xnacl, fsp);
  _fp->saturationTwoPhase(p, T, xnacl, z, fsp);
  ABS_TEST("gas saturation", fsp[1].saturation, s, 1.0e-8);
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
  Real xnacl = 0.1;

  Real Kh, dKh_dT, dKh_dx;
  _fp->henryConstant(T, xnacl, Kh, dKh_dT, dKh_dx);
  REL_TEST("Henry constant", Kh, 7.46559e+08, 1.0e-3);

  T = 473.15;
  xnacl = 0.2;

  _fp->henryConstant(T, xnacl, Kh, dKh_dT, dKh_dx);
  REL_TEST("Henry constant", Kh, 1.66069e+09, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real Kh2, dKh2_dT, dKh2_dx;
  _fp->henryConstant(T + dT, xnacl, Kh, dKh_dT, dKh_dx);
  _fp->henryConstant(T - dT, xnacl, Kh2, dKh2_dT, dKh2_dx);

  REL_TEST("dKh_dT", dKh_dT, (Kh - Kh2) / (2.0 * dT), 1.0e-5);

  // Test the derivative wrt xnacl
  const Real dx = 1.0e-8;
  _fp->henryConstant(T, xnacl + dx, Kh, dKh_dT, dKh_dx);
  _fp->henryConstant(T, xnacl - dx, Kh2, dKh2_dT, dKh2_dx);

  REL_TEST("dKh_dx", dKh_dx, (Kh - Kh2) / (2.0 * dx), 1.0e-5);
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
  Real xnacl = 0.1;

  // Enthalpy of dissolution of CO2 in brine
  Real hdis, dhdis_dT, dhdis_dx;
  _fp->enthalpyOfDissolutionGas(T, xnacl, hdis, dhdis_dT, dhdis_dx);
  REL_TEST("hdis", hdis, -3.20130e5, 1.0e-3);

  // T = 350C
  T = 623.15;

  // Enthalpy of dissolution of CO2 in brine
  _fp->enthalpyOfDissolutionGas(T, xnacl, hdis, dhdis_dT, dhdis_dx);
  REL_TEST("hdis", hdis, 9.83813e+05, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1, dhdis1_dT, dhdis1_dx, hdis2, dhdis2_dT, dhdis2_dx;
  _fp->enthalpyOfDissolutionGas(T + dT, xnacl, hdis1, dhdis1_dT, dhdis1_dx);
  _fp->enthalpyOfDissolutionGas(T - dT, xnacl, hdis2, dhdis2_dT, dhdis2_dx);

  REL_TEST("dhdis_dT", dhdis_dT, (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);

  // Test the derivative wrt salt mass fraction
  const Real dx = 1.0e-8;
  _fp->enthalpyOfDissolutionGas(T, xnacl + dx, hdis1, dhdis1_dT, dhdis1_dx);
  _fp->enthalpyOfDissolutionGas(T, xnacl - dx, hdis2, dhdis2_dT, dhdis2_dx);

  REL_TEST("dhdis_dx", dhdis_dx, (hdis1 - hdis2) / (2.0 * dx), 1.0e-5);
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
  REL_TEST("hdis", hdis, -3.38185e5, 1.0e-3);

  // T = 350C
  T = 623.15;

  // Enthalpy of dissolution of CO2 in water
  _fp->enthalpyOfDissolution(T, hdis, dhdis_dT);
  REL_TEST("hdis", hdis, 5.78787e5, 1.0e-3);

  // Test the derivative wrt temperature
  const Real dT = 1.0e-4;
  Real hdis1, dhdis1_dT, hdis2, dhdis2_dT;
  _fp->enthalpyOfDissolution(T + dT, hdis1, dhdis1_dT);
  _fp->enthalpyOfDissolution(T - dT, hdis2, dhdis2_dT);

  REL_TEST("dhdis_dT", dhdis_dT, (hdis1 - hdis2) / (2.0 * dT), 1.0e-5);
}

/**
 * Test iterative calculation of equilibrium mass fractions in the elevated
 * temperature regime
 */
TEST_F(PorousFlowBrineCO2Test, solveEquilibriumHighTemp)
{
  Real p = 20.0e6;
  Real T = 473.15;

  Real yh2o, xco2;

  _fp->solveEquilibriumHighTemp(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 0.161429471353, 1.0e-10);
  ABS_TEST("xco2", xco2, 0.0236967010229, 1.0e-10);

  p = 30.0e6;
  T = 523.15;

  _fp->solveEquilibriumHighTemp(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 0.286116587269, 1.0e-10);
  ABS_TEST("xco2", xco2, 0.0409622847096, 1.0e-10);
}

/**
 * Test calculation of equilibrium mole fractions over entire temperature range
 */
TEST_F(PorousFlowBrineCO2Test, equilibriumMoleFractions)
{
  // Low temperature regime
  Real p = 20.0e6;
  Real T = 323.15;

  Real yh2o, xco2;

  _fp->equilibriumMoleFractions(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 0.00696382327418, 1.0e-8);
  ABS_TEST("xco2", xco2, 0.0236534984353, 1.0e-8);

  // Intermediate temperature regime
  T = 373.15;

  _fp->equilibriumMoleFractions(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 0.0192345278072, 1.0e-8);
  ABS_TEST("xco2", xco2, 0.020193674787, 1.0e-8);

  // High temperature regime
  p = 30.0e6;
  T = 523.15;

  _fp->equilibriumMoleFractions(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 0.286116587269, 1.0e-8);
  ABS_TEST("xco2", xco2, 0.0409622847096, 1.0e-8);

  // High temperature regime with low pressure
  p = 1.0e6;
  T = 523.15;

  _fp->equilibriumMoleFractions(p, T, xco2, yh2o);
  ABS_TEST("yh2o", yh2o, 1.0, 1.0e-10);
  ABS_TEST("xco2", xco2, 0.0, 1.0e-10);
}

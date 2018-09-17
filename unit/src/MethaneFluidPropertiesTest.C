//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MethaneFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesPTTestUtils.h"

/**
 * Verify that critical properties are correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 4.5992e6, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalTemperature(), 190.564, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->criticalDensity(), 162.66, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that triple point properties are correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 1.169e4, REL_TOL_SAVED_VALUE);
  ABS_TEST(_fp->triplePointTemperature(), 90.67, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "methane"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(MethaneFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 16.0425e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of Henry's constant using data from
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(MethaneFluidPropertiesTest, henry)
{
  REL_TEST(_fp->henryConstant(300.0), 4069.0e6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->henryConstant(400.0), 6017.1e6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->henryConstant(500.0), 2812.9e6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->henryConstant(600.0), 801.8e6, REL_TOL_EXTERNAL_VALUE);
}

/**
 * Verify calculation of thermophysical properties of methane using
 * verification data provided in
 * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
 * Computer Equations
 */
TEST_F(MethaneFluidPropertiesTest, properties)
{
  // Pressure = 10 MPa, temperature = 350 K
  Real p = 10.0e6;
  Real T = 350.0;
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  REL_TEST(_fp->rho_from_p_T(p, T), 55.13, tol);
  REL_TEST(_fp->h_from_p_T(p, T), 708.5e3, tol);
  REL_TEST(_fp->e_from_p_T(p, T), 527.131e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 11.30e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 2.375e3, tol);
  REL_TEST(_fp->cv_from_p_T(p, T), 1.857e3, tol);
  REL_TEST(_fp->c_from_p_T(p, T), 481.7, tol);
  REL_TEST(_fp->mu_from_p_T(p, T), 0.01276e-3, tol);
  REL_TEST(_fp->k_from_p_T(p, T), 0.04113, tol);

  // Test s, h and cp for temperatures > 755K as well as these methods have a
  // different formulation in this regime
  T = 800.0;

  REL_TEST(_fp->h_from_p_T(p, T), 2132.0e3, tol);
  REL_TEST(_fp->s_from_p_T(p, T), 13.83e3, tol);
  REL_TEST(_fp->cp_from_p_T(p, T), 3.934e3, tol);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(MethaneFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 10.0e6;
  Real T = 350.0;

  DERIV_TEST(_fp->rho, _fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu, _fp->mu_dpT, p, T, tol);
  DERIV_TEST(_fp->e, _fp->e_dpT, p, T, tol);
  DERIV_TEST(_fp->h, _fp->h_dpT, p, T, tol);
  DERIV_TEST(_fp->k, _fp->k_dpT, p, T, tol);

  // Test derivative of enthalpy for T > 755 as well as it has a different formulation
  T = 800.0;
  DERIV_TEST(_fp->h, _fp->h_dpT, p, T, tol);

  // Henry's constant
  T = 350.0;
  const Real dT = 1.0e-4;

  Real dKh_dT_fd = (_fp->henryConstant(T + dT) - _fp->henryConstant(T - dT)) / (2.0 * dT);
  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant_dT(T, Kh, dKh_dT);
  REL_TEST(Kh, _fp->henryConstant(T), REL_TOL_SAVED_VALUE);
  REL_TEST(dKh_dT_fd, dKh_dT, REL_TOL_DERIVATIVE);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(MethaneFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;

  combinedProperties(_fp, p, T, REL_TOL_SAVED_VALUE);
}

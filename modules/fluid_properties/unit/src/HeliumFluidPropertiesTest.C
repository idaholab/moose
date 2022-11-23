//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "HeliumFluidPropertiesTest.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(HeliumFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "helium"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(HeliumFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 4.002602e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, thermalConductivity)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->k_from_v_e(v, e), 1.8651584722911332e-01, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->k_from_p_T(p, T), 1.8651584722911332e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the viscosity and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, viscosity)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->mu_from_v_e(v, e), 2.4061901685126415e-05, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->mu_from_p_T(p, T), 2.4061901685126415e-05, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, isobaricSpecificHeat)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->cp_from_p_T(p, T), 5195.0, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_v_e(v, e), 5195.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isochoric specific heat is correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, isochoricSpecificHeat)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->cv_from_v_e(v, e), 3117.0, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cv_from_p_T(p, T), 3117.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cv_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cv_from_v_e, v, e, REL_TOL_DERIVATIVE);
}

/**
 * Test that the density and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, density)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;

  ABS_TEST(_fp->rho_from_p_T(p, T), 1.2402629878970449e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific internal energy and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, specificInternalEnergy)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;

  ABS_TEST(_fp->e_from_p_T(p, T), 1.2254485499999998e6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the pressure and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, pressure)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->p_from_v_e(v, e), p, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);
}

/**
 * Test that the temperature and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, temperature)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;
  const Real e = _fp->e_from_p_T(p, T);
  const Real v = 1. / _fp->rho_from_p_T(p, T);

  REL_TEST(_fp->T_from_v_e(v, e), T, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);

  const Real h = _fp->h_from_p_T(p, T);
  REL_TEST(_fp->T_from_p_h(p, h), T, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the specific enthalpy and its derivatives are correctly computed.
 */
TEST_F(HeliumFluidPropertiesTest, specificEnthalpy)
{
  const Real T = 120.0 + 273.15;
  const Real p = 101325.0;

  ABS_TEST(_fp->h_from_p_T(p, T), 2.0424142499999998e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

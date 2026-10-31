//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LithiumFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(LithiumFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "lithium"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(LithiumFluidPropertiesTest, molarMass)
{
  REL_TEST(_fp->molarMass(), 6.941e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the lithium fluid properties
 */
TEST_F(LithiumFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const std::vector<Real> pressures = {1e5, 1e6, 5e6};
  const std::vector<Real> temperatures = {500, 1000, 1500};

  // Solutions: obtained from the correlations in NASA TN D-4650
  const std::vector<Real> rho_refs = {512.0, 462.0, 412.0};
  const std::vector<Real> h_refs = {1332600.0, 3417100.0, 5501600.0};
  const std::vector<Real> e_refs = {1332404.6875, 3414935.497835498, 5489464.077669903};
  const std::vector<Real> k_refs = {45.42025, 59.804, 65.02525};
  const std::vector<Real> mu_refs = {
      5.979432945653252e-4, 2.886888430248677e-4, 1.5227198177923657e-4};

  for (auto i : make_range(pressures.size()))
  {
    const Real p = pressures[i];
    const Real T = temperatures[i];
    const Real rho = _fp->rho_from_p_T(p, T);
    const Real h = _fp->h_from_p_T(p, T);

    // Density
    REL_TEST(_fp->rho_from_p_T(p, T), rho_refs[i], tol);

    // Specific volume
    REL_TEST(_fp->v_from_p_T(p, T), 1. / rho_refs[i], tol);

    // Enthalpy
    REL_TEST(_fp->h_from_p_T(p, T), h_refs[i], tol);

    // Specific energy
    REL_TEST(_fp->e_from_p_T(p, T), e_refs[i], tol);
    REL_TEST(_fp->e_from_p_rho(p, rho), e_refs[i], tol);

    // Temperature
    REL_TEST(_fp->T_from_p_rho(p, rho), T, tol);
    REL_TEST(_fp->T_from_p_h(p, h), T, tol);

    // Thermal conductivity
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], tol);

    // Isobaric and isochoric specific heat, constant and equal for this incompressible model
    REL_TEST(_fp->cp_from_p_T(p, T), 4169., tol);
    REL_TEST(_fp->cv_from_p_T(p, T), 4169., tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], tol);
  }
}

/**
 * Verify the values at the melting point against Table I of NASA TN D-4650
 */
TEST_F(LithiumFluidPropertiesTest, meltingPoint)
{
  const Real p = 1e5;
  const Real T = 453.7;

  REL_TEST(_fp->rho_from_p_T(p, T), 516., 2e-3);
  REL_TEST(_fp->h_from_p_T(p, T), 1.140e6, 1e-3);
  REL_TEST(_fp->k_from_p_T(p, T), 44.0, 1e-2);
  REL_TEST(_fp->mu_from_p_T(p, T), 0.645e-3, 1e-2);
}

/**
 * Verify calculation of the derivatives of lithium properties by comparing with finite
 * differences
 */
TEST_F(LithiumFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30.0e6;
  const Real T = 700.0;
  const Real T2 = 1200.;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T2, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T2, tol);
}

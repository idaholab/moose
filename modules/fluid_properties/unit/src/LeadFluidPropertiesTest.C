//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(LeadFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "Lead"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(LeadFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 207.2, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the Lead fluid properties
 */
TEST_F(LeadFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> pressures = {1e5, 1e6, 5e6};
  const std::vector<Real> temperatures = {700, 800, 1000};

  // Solutions : obtained from running the property object itself
  // Verified visually against the reference
  const std::vector<Real> rho_refs = {10545.4, 10417.4, 10161.5};
  const std::vector<Real> h_refs = {14622, 29147.4, 57656.6};
  const std::vector<Real> e_refs = {14612.5, 29051.4, 57164.5};
  const std::vector<Real> k_refs = {16.9, 18, 20.2};
  const std::vector<Real> E_refs = {3.34308e+10, 3.21221e+10, 2.9602e+10};
  const std::vector<Real> c_refs = {1780.8, 1756.2, 1707};
  const std::vector<Real> cp_refs = {146.194, 144.316, 140.886};
  const std::vector<Real> cv_refs = {119.492, 114.732, 106.102};
  const std::vector<Real> mu_refs = {0.00209528, 0.00173116, 0.00132517};

  for (auto i : make_range(pressures.size()))
  {
    // Obtain variable sets
    const Real p = pressures[i];
    const Real T = temperatures[i];
    const Real rho = _fp->rho_from_p_T(p, T);
    const Real v = 1. / rho;
    const Real e = _fp->e_from_p_T(p, T);

    // Density
    REL_TEST(_fp->rho_from_p_T(p, T), rho_refs[i], 10 * tol);

    // Enthalpy
    ABS_TEST(_fp->h_from_p_T(p, T), h_refs[i], 100 * tol);
    ABS_TEST(_fp->h_from_v_e(v, e), h_refs[i], 100 * tol);

    // Specific energy
    ABS_TEST(_fp->e_from_p_T(p, T), e_refs[i], 50 * tol);
    ABS_TEST(_fp->e_from_p_rho(p, rho), e_refs[i], 50 * tol);

    // Temperature
    ABS_TEST(_fp->T_from_v_e(v, e), T, tol);
    ABS_TEST(_fp->T_from_p_rho(p, rho), T, tol);

    // Thermal conductivity (function of T only)
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], 10 * tol);
    REL_TEST(_fp->k_from_v_e(v, e), k_refs[i], 10 * tol);

    // Bulk modulus
    REL_TEST(_fp->bulk_modulus_from_p_T(p, T), E_refs[i], tol);

    // Speed of sound
    REL_TEST(_fp->c_from_v_e(v, e), c_refs[i], 10 * tol);

    // Isobaric specific heat
    REL_TEST(_fp->cp_from_p_T(p, T), cp_refs[i], 10 * tol);
    REL_TEST(_fp->cp_from_v_e(v, e), cp_refs[i], 10 * tol);

    // Isochoric specific heat
    REL_TEST(_fp->cv_from_p_T(p, T), cv_refs[i], 10 * tol);
    REL_TEST(_fp->cv_from_v_e(v, e), cv_refs[i], 10 * tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], 10 * tol);
    REL_TEST(_fp->mu_from_v_e(v, e), mu_refs[i], 10 * tol);
  }
}

/**
 * Verify calculation of the derivatives of lead properties by comparing with finite
 * differences
 */
TEST_F(LeadFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30.0e6;
  const Real T = 300.0;
  const Real rho = _fp->rho_from_p_T(p, T);
  const Real v = 1. / rho;
  const Real e = _fp->e_from_p_T(p, T);

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cv_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);

  DERIV_TEST(_fp->p_from_v_e, v, e, tol);
  DERIV_TEST(_fp->mu_from_v_e, v, e, tol);
  DERIV_TEST(_fp->k_from_v_e, v, e, tol);
  DERIV_TEST(_fp->h_from_v_e, v, e, tol);
  DERIV_TEST(_fp->T_from_v_e, v, e, tol);
  DERIV_TEST(_fp->cp_from_v_e, v, e, tol);
  DERIV_TEST(_fp->cv_from_v_e, v, e, tol);

  DERIV_TEST(_fp->T_from_p_rho, p, rho, tol);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, tol);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadBismuthFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(LeadBismuthFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "LeadBismuth"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(LeadBismuthFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 233.99, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the LeadBismuth fluid properties
 */
TEST_F(LeadBismuthFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;
  const std::vector<Real> pressures = {1e5, 1e6, 5e6};
  const std::vector<Real> temperatures = {400, 500, 700};

  // Solutions : obtained from running the property object itself
  // Verified visually against the reference
  const std::vector<Real> rho_refs = {10547.8, 10418.5, 10159.9};
  const std::vector<Real> h_refs = {296.41, 15029.6, 43909.3};
  const std::vector<Real> e_refs = {286.93, 14933.6, 43417.2};
  const std::vector<Real> k_refs = {9.3832, 10.7927, 13.4736};
  const std::vector<Real> c_refs = {1770.2, 1749, 1706.6};
  const std::vector<Real> E_refs = {3.30472e+10, 3.187e+10, 2.95948e+10};
  const std::vector<Real> cp_refs = {148.19, 146.401, 142.414};
  const std::vector<Real> cv_refs = {131.481, 126.109, 115.606};
  const std::vector<Real> mu_refs = {0.00325447, 0.00223218, 0.00145073};

  for (auto i : make_range(pressures.size()))
  {
    // Obtain variable sets
    const Real p = pressures[i];
    const Real T = temperatures[i];
    const Real rho = _fp->rho_from_p_T(p, T);
    const Real v = 1. / rho;
    const Real e = _fp->e_from_p_T(p, T);

    // Density
    REL_TEST(_fp->rho_from_p_T(p, T), rho_refs[i], tol);

    // Enthalpy
    ABS_TEST(_fp->h_from_p_T(p, T), h_refs[i], 100 * tol);
    ABS_TEST(_fp->h_from_v_e(v, e), h_refs[i], 100 * tol);

    // Specific energy
    ABS_TEST(_fp->e_from_p_T(p, T), e_refs[i], 10 * tol);
    ABS_TEST(_fp->e_from_p_rho(p, rho), e_refs[i], 10 * tol);

    // Temperature
    ABS_TEST(_fp->T_from_v_e(v, e), T, tol);
    ABS_TEST(_fp->T_from_p_rho(p, rho), T, tol);

    // Thermal conductivity (function of T only)
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], 10.0 * tol);
    REL_TEST(_fp->k_from_v_e(v, e), k_refs[i], 10.0 * tol);

    // Bulk modulus
    REL_TEST(_fp->bulk_modulus_from_p_T(p, T), E_refs[i], 10.0 * tol);

    // Speed of sound
    REL_TEST(_fp->c_from_v_e(v, e), c_refs[i], 10.0 * tol);

    // Isobaric specific heat
    REL_TEST(_fp->cp_from_p_T(p, T), cp_refs[i], 10.0 * tol);
    REL_TEST(_fp->cp_from_v_e(v, e), cp_refs[i], 10.0 * tol);

    // Isochoric specific heat
    REL_TEST(_fp->cv_from_p_T(p, T), cv_refs[i], 10.0 * tol);
    REL_TEST(_fp->cv_from_v_e(v, e), cv_refs[i], 10.0 * tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], 10.0 * tol);
    REL_TEST(_fp->mu_from_v_e(v, e), mu_refs[i], 10.0 * tol);
  }
}

/**
 * Verify calculation of the derivatives of LeadBismuth properties by comparing with finite
 * differences
 */
TEST_F(LeadBismuthFluidPropertiesTest, derivatives)
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

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SalineMoltenSaltFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

#ifdef SALINE_ENABLED
/**
 * Test that the fluid name is correctly returned
 */
TEST_F(SalineMoltenSaltFluidPropertiesTest, fluidName)
{
  EXPECT_EQ(_fp->fluidName(), "LiF-NaF-KF");
}

/**
 * Verify calculation of the Saline fluid properties
 */
TEST_F(SalineMoltenSaltFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const Real very_large_tol = 1e-5;
  const std::vector<Real> pressures = {1e5, 1e6, 5e6};
  const std::vector<Real> temperatures = {800, 1000, 1200};

  // Solutions : obtained from running the property object itself
  // Verified visually against the reference
  const std::vector<Real> rho_refs = {2079.8, 1955, 1830.2};
  const std::vector<Real> h_refs = {116487.252639201, 503071.221504012, 932163.345918835};
  const std::vector<Real> k_refs = {0.69, 0.95, 1.21};
  const std::vector<Real> cp_refs = {1826.64945544902, 2039.19023319908, 2251.73101094914};
  const std::vector<Real> mu_refs = {0.00664316439747037, 0.00230674718872007, 0.00141416472507015};

  for (auto i : make_range(pressures.size()))
  {
    // Obtain variable sets
    const Real p = pressures[i];
    const Real T = temperatures[i];
    const Real h = _fp->h_from_p_T(p, T);

    // Density
    REL_TEST(_fp->rho_from_p_T(p, T), rho_refs[i], tol);

    // Specific enthalpy
    REL_TEST(_fp->h_from_p_T(p, T), h_refs[i], tol);

    // Specific energy
    REL_TEST(_fp->e_from_p_T(p, T), h_refs[i] - pressures[i] / rho_refs[i], tol);

    // Temperature
    // Does not seem very accurate
    REL_TEST(_fp->T_from_p_h(p, h), T, very_large_tol);

    // Thermal conductivity (function of T only)
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], tol);

    // Isobaric specific heat
    REL_TEST(_fp->cp_from_p_T(p, T), cp_refs[i], tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], tol);
  }
}

/**
 * Verify calculation of the derivatives of Saline properties by comparing with finite
 * differences
 */
TEST_F(SalineMoltenSaltFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30.0e6;
  const Real T = 300.0;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, 100 * tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
}
#endif

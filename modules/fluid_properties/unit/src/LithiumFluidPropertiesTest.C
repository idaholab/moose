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
  const std::vector<Real> temperatures = {500, 1000, 1400};

  // Solutions: obtained from the correlations of Ohse (1985) as tabulated by Zinkle (1998)
  const std::vector<Real> rho_refs = {510.7417563927444, 466.6006305350272, 429.6222453460765};
  const std::vector<Real> h_refs = {201435.71359199984, 2316435.713592, 3943203.713592};
  const std::vector<Real> e_refs = {201239.91992769035, 2314292.553179093, 3931565.5825264812};
  const std::vector<Real> cp_refs = {4364.25, 4120.0, 4029.36};
  const std::vector<Real> k_refs = {44.1725, 59.85, 67.9172};
  const std::vector<Real> mu_refs = {
      5.308711604055202e-4, 2.5483272337520357e-4, 1.8917694550102165e-4};

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

    // Temperature, through the Newton inversions of the density and enthalpy correlations
    REL_TEST(_fp->T_from_p_rho(p, rho), T, tol);
    REL_TEST(_fp->T_from_p_h(p, h), T, tol);

    // Thermal conductivity
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], tol);

    // Isobaric and isochoric specific heat, equal for this incompressible model
    REL_TEST(_fp->cp_from_p_T(p, T), cp_refs[i], tol);
    REL_TEST(_fp->cv_from_p_T(p, T), cp_refs[i], tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], tol);
  }
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
  DERIV_TEST(_fp->rho_from_p_T, p, T2, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T2, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T2, tol);
}

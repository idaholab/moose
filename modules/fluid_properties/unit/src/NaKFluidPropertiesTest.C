//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaKFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(NaKFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "NaK"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(NaKFluidPropertiesTest, molarMass)
{
  REL_TEST(_fp->molarMass(), 0.0620918, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the NaK fluid properties
 */
TEST_F(NaKFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const std::vector<Real> pressures = {1e5, 1e6, 5e6};
  const std::vector<Real> temperatures = {500, 700, 1000};

  // Solutions : obtained from running the property object itself
  // Verified visually against the reference
  const std::vector<Real> rho_refs = {
      821.954085329590612, 775.608846516464268, 703.767342628214124};
  const std::vector<Real> h_refs = {212851.212833445577, 391103.66397524561, 654472.411687945598};
  const std::vector<Real> e_refs = {212729.55153656195, 389814.354287132388, 647367.791031170636};
  const std::vector<Real> k_refs = {24.9636547049999997, 26.2273747049999955, 24.8229547050000008};
  const std::vector<Real> cp_refs = {908.088827709000043, 879.02768370900003, 887.095967709000092};
  const std::vector<Real> cv_refs = {908.055029910277312, 878.637048045562551, 884.635754614549114};
  const std::vector<Real> mu_refs = {
      0.000336716758684162674, 0.000222908484361340899, 0.000145271584941717932};

  for (auto i : make_range(pressures.size()))
  {
    // Obtain variable sets
    const Real p = pressures[i];
    const Real T = temperatures[i];
    const Real rho = _fp->rho_from_p_T(p, T);

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

    // Thermal conductivity (function of T only)
    REL_TEST(_fp->k_from_p_T(p, T), k_refs[i], tol);

    // Isobaric specific heat
    REL_TEST(_fp->cp_from_p_T(p, T), cp_refs[i], tol);

    // Isochoric specific heat
    REL_TEST(_fp->cv_from_p_T(p, T), cv_refs[i], tol);

    // Dynamic viscosity
    REL_TEST(_fp->mu_from_p_T(p, T), mu_refs[i], tol);
  }
}

/**
 * Verify calculation of the derivatives of NaK properties by comparing with finite
 * differences
 */
TEST_F(NaKFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30.0e6;
  const Real T = 400.0; // K
  const Real T2 = 700.;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T2, tol);
}

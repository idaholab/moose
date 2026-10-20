//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedPVTFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "MooseException.h"

/**
 * Test that the fluid name and molar mass are correctly returned
 */
TEST_F(TabulatedPVTFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "tabulated_pvt"); }

TEST_F(TabulatedPVTFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), _molar_mass, REL_TOL_SAVED_VALUE);
}

/**
 * At the tabulated pressures, density and viscosity must reproduce the table exactly
 */
TEST_F(TabulatedPVTFluidPropertiesTest, tableValues)
{
  const Real T = 350.0;
  for (std::size_t i = 0; i < _p.size(); ++i)
  {
    REL_TEST(_fp->rho_from_p_T(_p[i], T), _rho_sc / _B[i], REL_TOL_CONSISTENCY);
    REL_TEST(_fp->mu_from_p_T(_p[i], T), _mu[i], REL_TOL_CONSISTENCY);
    REL_TEST(_fp_bounds->rho_from_p_T(_p[i], T), _rho_sc / _B[i], REL_TOL_CONSISTENCY);
    REL_TEST(_fp_bounds->mu_from_p_T(_p[i], T), _mu[i], REL_TOL_CONSISTENCY);
    REL_TEST(_fp_throw->rho_from_p_T(_p[i], T), _rho_sc / _B[i], REL_TOL_CONSISTENCY);
    REL_TEST(_fp_throw->mu_from_p_T(_p[i], T), _mu[i], REL_TOL_CONSISTENCY);
  }
}

/**
 * Between tabulated pressures, 1/B and 1/(B mu) are interpolated linearly
 */
TEST_F(TabulatedPVTFluidPropertiesTest, interpolation)
{
  const Real T = 350.0;
  const Real p = 1.5e7; // midpoint of the second interval

  const Real inv_B = 0.5 * (1.0 / _B[1] + 1.0 / _B[2]);
  const Real inv_B_mu = 0.5 * (1.0 / (_B[1] * _mu[1]) + 1.0 / (_B[2] * _mu[2]));
  const Real rho = _rho_sc * inv_B;
  const Real mu = inv_B / inv_B_mu;

  REL_TEST(_fp->rho_from_p_T(p, T), rho, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->mu_from_p_T(p, T), mu, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->e_from_p_T(p, T), _cv * T, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->h_from_p_T(p, T), _cv * T + p / rho, REL_TOL_CONSISTENCY);

  // Saved values, also used in the regression test fluids/tabulated_pvt.i
  REL_TEST(_fp->rho_from_p_T(p, T), 814.20313790256, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->mu_from_p_T(p, T), 1.1341216986149e-3, REL_TOL_SAVED_VALUE);

  // rho_mu_from_p_T (used by PorousFlow) must be consistent with the individual methods
  Real rho2, mu2;
  _fp->rho_mu_from_p_T(p, T, rho2, mu2);
  REL_TEST(rho2, rho, REL_TOL_CONSISTENCY);
  REL_TEST(mu2, mu, REL_TOL_CONSISTENCY);
}

/**
 * With out_of_bounds_behavior = ignore, 1/B and 1/(B mu) are extrapolated linearly from the end
 * intervals outside the table
 */
TEST_F(TabulatedPVTFluidPropertiesTest, extrapolation)
{
  const Real T = 350.0;

  // Above the table: half an interval beyond the last tabulated pressure
  {
    const Real p = 3.5e7;
    const Real inv_B = 1.0 / _B[3] + 0.5 * (1.0 / _B[3] - 1.0 / _B[2]);
    const Real inv_B_mu =
        1.0 / (_B[3] * _mu[3]) + 0.5 * (1.0 / (_B[3] * _mu[3]) - 1.0 / (_B[2] * _mu[2]));
    REL_TEST(_fp->rho_from_p_T(p, T), _rho_sc * inv_B, REL_TOL_CONSISTENCY);
    REL_TEST(_fp->mu_from_p_T(p, T), inv_B / inv_B_mu, REL_TOL_CONSISTENCY);
  }

  // Below the table: an eighth of the first interval below the first tabulated pressure
  {
    const Real p = 1.0e6;
    const Real inv_B = 1.0 / _B[0] - 0.125 * (1.0 / _B[1] - 1.0 / _B[0]);
    const Real inv_B_mu =
        1.0 / (_B[0] * _mu[0]) - 0.125 * (1.0 / (_B[1] * _mu[1]) - 1.0 / (_B[0] * _mu[0]));
    REL_TEST(_fp->rho_from_p_T(p, T), _rho_sc * inv_B, REL_TOL_CONSISTENCY);
    REL_TEST(_fp->mu_from_p_T(p, T), inv_B / inv_B_mu, REL_TOL_CONSISTENCY);
  }
}

/**
 * With out_of_bounds_behavior = set_to_closest_bound, the end values are used outside the table
 */
TEST_F(TabulatedPVTFluidPropertiesTest, setToClosestBound)
{
  const Real T = 350.0;
  Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;

  _fp_bounds->rho_from_p_T(3.5e7, T, rho, drho_dp, drho_dT);
  _fp_bounds->mu_from_p_T(3.5e7, T, mu, dmu_dp, dmu_dT);
  REL_TEST(rho, _rho_sc / _B[3], REL_TOL_CONSISTENCY);
  REL_TEST(mu, _mu[3], REL_TOL_CONSISTENCY);
  ABS_TEST(drho_dp, 0.0, REL_TOL_CONSISTENCY);
  ABS_TEST(dmu_dp, 0.0, REL_TOL_CONSISTENCY);

  _fp_bounds->rho_from_p_T(1.0e6, T, rho, drho_dp, drho_dT);
  _fp_bounds->mu_from_p_T(1.0e6, T, mu, dmu_dp, dmu_dT);
  REL_TEST(rho, _rho_sc / _B[0], REL_TOL_CONSISTENCY);
  REL_TEST(mu, _mu[0], REL_TOL_CONSISTENCY);
  ABS_TEST(drho_dp, 0.0, REL_TOL_CONSISTENCY);
  ABS_TEST(dmu_dp, 0.0, REL_TOL_CONSISTENCY);
}

/**
 * Verify the derivatives against finite differences, inside the table and in the
 * extrapolated regions. Pressures are chosen away from the tabulated points, where
 * the piecewise-linear interpolant has kinks.
 */
TEST_F(TabulatedPVTFluidPropertiesTest, derivatives)
{
  const Real T = 350.0;
  const Real tol = REL_TOL_DERIVATIVE;

  for (const Real p : {1.0e6, 5.0e6, 1.5e7, 2.5e7, 3.5e7})
  {
    DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
    DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
    DERIV_TEST(_fp->e_from_p_T, p, T, tol);
    DERIV_TEST(_fp->h_from_p_T, p, T, tol);

    DERIV_TEST(_fp_bounds->rho_from_p_T, p, T, tol);
    DERIV_TEST(_fp_bounds->mu_from_p_T, p, T, tol);
    DERIV_TEST(_fp_bounds->h_from_p_T, p, T, tol);
  }

  // Inside the table, the default (throw) object behaves like the others
  for (const Real p : {5.0e6, 1.5e7, 2.5e7})
  {
    DERIV_TEST(_fp_throw->rho_from_p_T, p, T, tol);
    DERIV_TEST(_fp_throw->mu_from_p_T, p, T, tol);
    DERIV_TEST(_fp_throw->h_from_p_T, p, T, tol);
  }
}

/**
 * The AD versions (used by PorousFlow AD kernels) must carry the same derivatives
 */
TEST_F(TabulatedPVTFluidPropertiesTest, ad)
{
  const Real p = 1.5e7;
  const Real T = 350.0;

  ADReal ad_p = p;
  Moose::derivInsert(ad_p.derivatives(), 0, 1.0);
  ADReal ad_T = T;
  Moose::derivInsert(ad_T.derivatives(), 1, 1.0);

  Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);

  const ADReal ad_rho = _fp->rho_from_p_T(ad_p, ad_T);
  const ADReal ad_mu = _fp->mu_from_p_T(ad_p, ad_T);

  REL_TEST(ad_rho.value(), rho, REL_TOL_CONSISTENCY);
  REL_TEST(ad_rho.derivatives()[0], drho_dp, REL_TOL_CONSISTENCY);
  ABS_TEST(ad_rho.derivatives()[1], drho_dT, REL_TOL_CONSISTENCY);
  REL_TEST(ad_mu.value(), mu, REL_TOL_CONSISTENCY);
  REL_TEST(ad_mu.derivatives()[0], dmu_dp, REL_TOL_CONSISTENCY);
  ABS_TEST(ad_mu.derivatives()[1], dmu_dT, REL_TOL_CONSISTENCY);
}

/**
 * With out_of_bounds_behavior = throw (the default), an exception is thrown outside the table so
 * that the time step can be cut, but not at the end points of the table
 */
TEST_F(TabulatedPVTFluidPropertiesTest, throwException)
{
  const Real T = 350.0;

  EXPECT_THROW(_fp_throw->rho_from_p_T(3.5e7, T), MooseException);
  EXPECT_THROW(_fp_throw->mu_from_p_T(3.5e7, T), MooseException);
  EXPECT_THROW(_fp_throw->h_from_p_T(3.5e7, T), MooseException);
  EXPECT_THROW(_fp_throw->rho_from_p_T(1.0e6, T), MooseException);
  EXPECT_THROW(_fp_throw->mu_from_p_T(1.0e6, T), MooseException);

  EXPECT_NO_THROW(_fp_throw->rho_from_p_T(_p.front(), T));
  EXPECT_NO_THROW(_fp_throw->rho_from_p_T(_p.back(), T));

  try
  {
    _fp_throw->rho_from_p_T(3.5e7, T);
    FAIL() << "missing expected exception";
  }
  catch (const MooseException & err)
  {
    EXPECT_NE(std::string(err.what()).find("is outside the range of tabulated pressure"),
              std::string::npos)
        << err.what();
  }
}

/**
 * With out_of_bounds_behavior = declare_invalid or warn_invalid, the end values are used outside
 * the table, as with set_to_closest_bound, and no exception is thrown
 */
TEST_F(TabulatedPVTFluidPropertiesTest, invalidSolution)
{
  const Real T = 350.0;

  for (const auto * fp : {_fp_declare_invalid, _fp_warn_invalid})
  {
    Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;

    fp->rho_from_p_T(3.5e7, T, rho, drho_dp, drho_dT);
    fp->mu_from_p_T(3.5e7, T, mu, dmu_dp, dmu_dT);
    REL_TEST(rho, _rho_sc / _B[3], REL_TOL_CONSISTENCY);
    REL_TEST(mu, _mu[3], REL_TOL_CONSISTENCY);
    ABS_TEST(drho_dp, 0.0, REL_TOL_CONSISTENCY);
    ABS_TEST(dmu_dp, 0.0, REL_TOL_CONSISTENCY);

    fp->rho_from_p_T(1.0e6, T, rho, drho_dp, drho_dT);
    fp->mu_from_p_T(1.0e6, T, mu, dmu_dp, dmu_dT);
    REL_TEST(rho, _rho_sc / _B[0], REL_TOL_CONSISTENCY);
    REL_TEST(mu, _mu[0], REL_TOL_CONSISTENCY);
  }
}

/**
 * Extrapolating far enough below the table makes 1/B non-positive: an exception must be
 * thrown so that the time step can be cut
 */
TEST_F(TabulatedPVTFluidPropertiesTest, extrapolationError)
{
  // 1/B reaches zero at about -7E8 Pa for this table
  EXPECT_THROW(_fp->rho_from_p_T(-1.0e9, 350.0), MooseException);
  EXPECT_THROW(_fp->mu_from_p_T(-1.0e9, 350.0), MooseException);
}

/**
 * Invalid tables must be rejected with an informative error
 */
TEST_F(TabulatedPVTFluidPropertiesTest, invalidTables)
{
  auto expect_error =
      [this](InputParameters params, const std::string & name, const std::string & msg)
  {
    try
    {
      _fe_problem->addUserObject("TabulatedPVTFluidProperties", name, params);
      FAIL() << "missing expected error: " << msg;
    }
    catch (const std::exception & err)
    {
      EXPECT_NE(std::string(err.what()).find(msg), std::string::npos) << err.what();
    }
  };

  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("pressure") = {2.0e6, 1.0e7, 1.0e7, 3.0e7};
    expect_error(params, "non_increasing", "Tabulated pressures must be strictly increasing");
  }
  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("pressure") = {2.0e6};
    params.set<std::vector<Real>>("formation_volume_factor") = {1.062};
    params.set<std::vector<Real>>("viscosity") = {1.04e-3};
    expect_error(params, "one_point", "At least two tabulated pressures are required");
  }
  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("formation_volume_factor") = {1.062, 1.050, 1.038};
    expect_error(params, "wrong_size_B", "Must have the same number of entries as 'pressure'");
  }
  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("viscosity") = {1.04e-3, 1.10e-3};
    expect_error(params, "wrong_size_mu", "Must have the same number of entries as 'pressure'");
  }
  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("formation_volume_factor") = {1.062, 0.0, 1.038, 1.028};
    expect_error(params, "zero_B", "All entries must be positive");
  }
  {
    InputParameters params = validFPParams();
    params.set<std::vector<Real>>("viscosity") = {1.04e-3, -1.10e-3, 1.17e-3, 1.24e-3};
    expect_error(params, "negative_mu", "All entries must be positive");
  }
}

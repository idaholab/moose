//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ElectromagneticCopperPropertiesTest.h"

/**
 * Test electrical resistivity and its derivatives at multiple temperatures.
 * Golden values computed from NIST zero-field copper electrical-resistivity correlation
 * using compute_electromagnetic_golden_values.C (machine precision, 17 significant digits).
 * Valid range: 2-900 K
 */
TEST_F(ElectromagneticCopperPropertiesTest, electrical_resistivity)
{
  Real T;

  // T = 2.0 K (minimum valid temperature)
  T = 2.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 1.55300382363870312e-10, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 77.0 K (liquid nitrogen temperature)
  T = 77.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 2.05748727925135603e-09, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 273.15 K (ice point, used for RRR definition)
  T = 273.15;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 1.55874299075489154e-08, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 300.0 K (room temperature)
  T = 300.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 1.73901817183488270e-08, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 900.0 K (maximum valid temperature)
  T = 900.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 6.09368093379040018e-08, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test electrical conductivity and its derivatives at multiple temperatures.
 * Golden values: sigma = 1/rho from NIST correlation (machine precision).
 */
TEST_F(ElectromagneticCopperPropertiesTest, electrical_conductivity)
{
  Real T;

  // T = 77.0 K
  T = 77.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 4.86029736409288168e+08, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 273.15 K
  T = 273.15;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 6.41542580098919943e+07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // T = 300.0 K
  T = 300.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 5.75037119333189204e+07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test magnetic permeability (constant property).
 * Golden value: mu_0 = 4π × 10^-7 H/m = 1.25663706212e-6 H/m
 */
TEST_F(ElectromagneticCopperPropertiesTest, magnetic_permeability)
{
  Real T;

  // Test at multiple temperatures - should be constant
  T = 2.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706212e-6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);

  T = 77.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706212e-6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);

  T = 300.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706212e-6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);

  T = 900.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706212e-6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test reciprocal relationship: sigma = 1/rho
 * This verifies numerical consistency between the two properties.
 */
TEST_F(ElectromagneticCopperPropertiesTest, resistivity_conductivity_reciprocal)
{
  Real T;

  // Test at several temperatures across the range
  T = 77.0;
  Real rho_77 = _sp->electrical_resistivity_from_T(T);
  Real sigma_77 = _sp->electrical_conductivity_from_T(T);
  REL_TEST(sigma_77 * rho_77, 1.0, REL_TOL_CONSISTENCY);

  T = 273.15;
  Real rho_273 = _sp->electrical_resistivity_from_T(T);
  Real sigma_273 = _sp->electrical_conductivity_from_T(T);
  REL_TEST(sigma_273 * rho_273, 1.0, REL_TOL_CONSISTENCY);

  T = 900.0;
  Real rho_900 = _sp->electrical_resistivity_from_T(T);
  Real sigma_900 = _sp->electrical_conductivity_from_T(T);
  REL_TEST(sigma_900 * rho_900, 1.0, REL_TOL_CONSISTENCY);
}

/**
 * Test RRR parameter effect on residual resistivity.
 * Higher RRR → lower residual resistivity → lower total resistivity at low T.
 */
TEST_F(ElectromagneticCopperPropertiesTest, rrr_effect)
{
  // Create objects with different RRR values
  InputParameters uo_pars_rrr50 = _factory.getValidParams("ElectromagneticCopperProperties");
  uo_pars_rrr50.set<Real>("rrr") = 50.0;
  _fe_problem->addUserObject("ElectromagneticCopperProperties", "sp_rrr50", uo_pars_rrr50);
  const auto & sp_rrr50 = _fe_problem->getUserObject<ElectromagneticCopperProperties>("sp_rrr50");

  InputParameters uo_pars_rrr300 = _factory.getValidParams("ElectromagneticCopperProperties");
  uo_pars_rrr300.set<Real>("rrr") = 300.0;
  _fe_problem->addUserObject("ElectromagneticCopperProperties", "sp_rrr300", uo_pars_rrr300);
  const auto & sp_rrr300 = _fe_problem->getUserObject<ElectromagneticCopperProperties>("sp_rrr300");

  // At low temperature (2K), resistivity dominated by residual resistivity
  Real T = 2.0;
  Real rho_50 = sp_rrr50.electrical_resistivity_from_T(T);
  Real rho_100 = _sp->electrical_resistivity_from_T(T); // Default RRR=100
  Real rho_300 = sp_rrr300.electrical_resistivity_from_T(T);

  // Higher RRR → lower resistivity
  EXPECT_LT(rho_300, rho_100);
  EXPECT_LT(rho_100, rho_50);

  // At room temperature, phonon scattering dominates, but residual still contributes
  T = 300.0;
  Real rho_300_50 = sp_rrr50.electrical_resistivity_from_T(T);
  Real rho_300_100 = _sp->electrical_resistivity_from_T(T);
  Real rho_300_300 = sp_rrr300.electrical_resistivity_from_T(T);

  // Effect is smaller at high T, but ordering should be preserved
  EXPECT_LT(rho_300_300, rho_300_100);
  EXPECT_LT(rho_300_100, rho_300_50);
}

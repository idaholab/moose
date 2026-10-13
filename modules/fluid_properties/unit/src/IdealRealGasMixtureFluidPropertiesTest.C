//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealRealGasMixtureFluidPropertiesTest.h"
#include "VaporMixtureFluidPropertiesUtils.h"
#include <cstdlib>

TEST_F(IdealRealGasMixtureFluidPropertiesTest, test)
{
  // save this setting so that it can be restored later
  const bool original_throw_on_warning = Moose::_throw_on_warning;
  Moose::_throw_on_warning = false;

  Real T = 400.;
  Real p = 100000.;
  std::vector<Real> x = {0.9};

  // pure fluids
  const Real rho_nitrogen = _fp_nitrogen->rho_from_p_T(p, T);
  const Real rho_steam = _fp_steam->rho_from_p_T(p, T);
  REL_TEST(rho_nitrogen, 0.84228968026683726, REL_TOL_SAVED_VALUE);
  REL_TEST(rho_steam, 0.61493738819320221, REL_TOL_SAVED_VALUE);

  // rho(p,T)
  const Real rho_mix = _fp_mix->rho_from_p_T(p, T, x);
  REL_TEST(rho_mix, 0.88183704292782583, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->rho_from_p_T, p, T, x, REL_TOL_DERIVATIVE);

  // v(p,T)
  const Real v_mix = _fp_mix->v_from_p_T(p, T, x);
  REL_TEST(v_mix, 1.1339963636363655, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->v_from_p_T, p, T, x, REL_TOL_DERIVATIVE);

  /// e(p,T)
  const Real e_mix = _fp_mix->e_from_p_T(p, T, x);
  REL_TEST(e_mix, 523068.96363636368, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // e(p,rho)
  REL_TEST(_fp_mix->e_from_p_rho(p, rho_mix, x), e_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_p_rho, p, rho_mix, x, REL_TOL_DERIVATIVE);
  // e(T,v)
  REL_TEST(_fp_mix->e_from_T_v(T, v_mix, x), e_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // p(v,e)
  REL_TEST(_fp_mix->p_from_v_e(v_mix, e_mix, x), p, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->p_from_v_e, v_mix, e_mix, x, REL_TOL_DERIVATIVE);
  // p(T,v)
  REL_TEST(_fp_mix->p_from_T_v(T, v_mix, x), p, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->p_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // T(v,e)
  REL_TEST(_fp_mix->T_from_v_e(v_mix, e_mix, x), T, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->T_from_v_e, v_mix, e_mix, x, REL_TOL_DERIVATIVE);
  // T(p,v)
  REL_TEST(_fp_mix->T_from_p_v(p, v_mix, x), T, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->T_from_p_v, p, v_mix, x, REL_TOL_DERIVATIVE);

  // c(p,T)
  const Real c_mix = _fp_mix->c_from_p_T(p, T, x);
  REL_TEST(c_mix, 418.49693534182865, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->c_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // c(v,e)
  REL_TEST(_fp_mix->c_from_v_e(v_mix, e_mix, x), c_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->c_from_v_e, v_mix, e_mix, x, REL_TOL_DERIVATIVE);

  /// s(p,T)
  const Real s_mix = _fp_mix->s_from_p_T(p, T, x);
  REL_TEST(s_mix, 700.961535541455, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->s_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // s(T,v)
  REL_TEST(_fp_mix->s_from_T_v(T, v_mix, x), s_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->s_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // cp(p,T)
  const Real cp_mix = _fp_mix->cp_from_p_T(p, T, x);
  REL_TEST(cp_mix, 1083.6715000000002, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cp_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // cp(T,v)
  REL_TEST(_fp_mix->cp_from_T_v(T, v_mix, x), cp_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cp_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // cv(p,T)
  const Real cv_mix = _fp_mix->cv_from_p_T(p, T, x);
  REL_TEST(cv_mix, 771.82250000000022, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cv_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // cv(T,v)
  REL_TEST(_fp_mix->cv_from_T_v(T, v_mix, x), cv_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cv_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // mu(p,T)
  const Real mu_mix = _fp_mix->mu_from_p_T(p, T, x);
  REL_TEST(mu_mix, 2.089269788624648e-05, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->mu_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // mu(T,v)
  REL_TEST(_fp_mix->mu_from_T_v(T, v_mix, x), mu_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->mu_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // k(p,T)
  const Real k_mix = _fp_mix->k_from_p_T(p, T, x);
  REL_TEST(k_mix, 0.0319250086967551, REL_TOL_SAVED_VALUE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->k_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // k(T,v)
  REL_TEST(_fp_mix->k_from_T_v(T, v_mix, x), k_mix, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->k_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // Compare calculations with a state-of-the-art mixture model. Note these are
  // not expected to be exactly the same, so very loose tolerances must be used.
  // mu and k are not available in the state-of-the-art mixture model.
  T = 360.;
  p = 100000.;
  x[0] = 0.5;
  REL_TEST(_fp_mix->rho_from_p_T(p, T, x), 0.7362935, 0.115554);
  REL_TEST(_fp_mix->v_from_p_T(p, T, x), 1.3581540, 0.103585);
  REL_TEST(_fp_mix->e_from_p_T(p, T, x), 1380586.9, 0.023648);
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), 428.1928, 0.017535);
  REL_TEST(_fp_mix->cp_from_p_T(p, T, x), 1479.7114, 0.146444);
  REL_TEST(_fp_mix->cv_from_p_T(p, T, x), 1090.5912, 0.183001);

  // restore original setting
  Moose::_throw_on_warning = original_throw_on_warning;
}

/**
 * Verify that the bracketing bounds computed from the primary component's critical point
 * belong to each mixture object rather than to the first one that was evaluated
 */
TEST_F(IdealRealGasMixtureFluidPropertiesTest, primaryCriticalPointPerObject)
{
  // Evaluate the fixture mixture first, so that any state shared between objects is set from it
  const std::vector<Real> x = {0.5};
  _fp_mix->v_from_p_T(1e5, 700.0, x);

  // A second mixture whose primary component has a much larger critical density
  {
    InputParameters params = _factory.getValidParams("StiffenedGasFluidProperties");
    params.set<Real>("gamma") = 1.43;
    params.set<Real>("q") = 2030e3;
    params.set<Real>("q_prime") = -23e3;
    params.set<Real>("p_inf") = 1e4;
    params.set<Real>("cv") = 1040;
    params.set<Real>("M") = 0.01801488;
    params.set<Real>("T_c") = 647.096;
    params.set<Real>("rho_c") = 1000.;
    params.set<Real>("e_c") = 2015734.52419;
    _fe_problem->addUserObject("StiffenedGasFluidProperties", "fp_dense_primary", params);
  }
  {
    InputParameters params = _factory.getValidParams("IdealRealGasMixtureFluidProperties");
    params.set<UserObjectName>("fp_primary") = "fp_dense_primary";
    params.set<std::vector<UserObjectName>>("fp_secondary") = {"fp_nitrogen"};
    params.set<bool>("allow_imperfect_jacobians") = true;
    _fe_problem->addUserObject("IdealRealGasMixtureFluidProperties", "fp_dense_mix", params);
  }
  const auto & fp_dense_mix =
      _fe_problem->getUserObject<IdealRealGasMixtureFluidProperties>("fp_dense_mix");

  // Supercritical state whose specific volume lies below the fixture primary's critical
  // specific volume times the primary mass fraction, but above that of the dense primary
  const Real T = 700.0;
  const Real p = 3e8;
  const Real v = fp_dense_mix.v_from_p_T(p, T, x);
  EXPECT_FALSE(std::isnan(v));
  REL_TEST(fp_dense_mix.p_from_T_v(T, v, x), p, REL_TOL_CONSISTENCY);
}

/**
 * Verify the isobaric heat capacity against (dh/dT)_p of the mixture enthalpy
 * h = sum_i x_i h_i(T, v / x_i), using a primary component whose enthalpy depends on
 * the specific volume
 */
TEST_F(IdealRealGasMixtureFluidPropertiesTest, cpVolumeDependentEnthalpy)
{
  const bool original_throw_on_warning = Moose::_throw_on_warning;
  Moose::_throw_on_warning = false;

  InputParameters fp_params = _factory.getValidParams("TestTVFluidProperties");
  _fe_problem->addUserObject("TestTVFluidProperties", "fp_tv", fp_params);
  InputParameters params = _factory.getValidParams("IdealRealGasMixtureFluidProperties");
  params.set<UserObjectName>("fp_primary") = "fp_tv";
  params.set<std::vector<UserObjectName>>("fp_secondary") = {"fp_nitrogen"};
  params.set<bool>("allow_imperfect_jacobians") = true;
  _fe_problem->addUserObject("IdealRealGasMixtureFluidProperties", "fp_tv_mix", params);
  const auto & fp_mix = _fe_problem->getUserObject<IdealRealGasMixtureFluidProperties>("fp_tv_mix");

  const Real T = 400.0;
  const Real v = 1.2;
  const std::vector<Real> x = {0.3};

  // Mixture enthalpy as a function of (T, v)
  const auto h_mix = [&](Real T, Real v)
  {
    const auto & fp_tv = _fe_problem->getUserObject<SinglePhaseFluidProperties>("fp_tv");
    return (1.0 - x[0]) * fp_tv.h_from_T_v(T, v / (1.0 - x[0])) +
           x[0] * _fp_nitrogen->h_from_T_v(T, v / x[0]);
  };
  const Real dT = 1e-4;
  const Real dv = 1e-7;
  const Real dh_dT = (h_mix(T + dT, v) - h_mix(T - dT, v)) / (2.0 * dT);
  const Real dh_dv = (h_mix(T, v + dv) - h_mix(T, v - dv)) / (2.0 * dv);
  Real p, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  fp_mix.p_from_T_v(T, v, x, p, dp_dT, dp_dv, dp_dx);
  const Real cp_ref = dh_dT - dh_dv * dp_dT / dp_dv;

  REL_TEST(fp_mix.cp_from_T_v(T, v, x), cp_ref, REL_TOL_DERIVATIVE);
  Real cp, dcp_dT, dcp_dv;
  std::vector<Real> dcp_dx;
  fp_mix.cp_from_T_v(T, v, x, cp, dcp_dT, dcp_dv, dcp_dx);
  REL_TEST(cp, cp_ref, REL_TOL_DERIVATIVE);

  Moose::_throw_on_warning = original_throw_on_warning;
}

/**
 * Verify that the value and derivative overloads of the transport properties agree when a
 * secondary component's internal energy depends on the specific volume
 */
TEST_F(IdealRealGasMixtureFluidPropertiesTest, transportVolumeDependentSecondary)
{
  const bool original_throw_on_warning = Moose::_throw_on_warning;
  Moose::_throw_on_warning = false;

  // Secondary component whose e(T, v) depends on v and whose transport properties depend on e
  InputParameters fp_params = _factory.getValidParams("TestTVFluidProperties");
  _fe_problem->addUserObject("TestTVFluidProperties", "fp_tv_secondary", fp_params);
  InputParameters params = _factory.getValidParams("IdealRealGasMixtureFluidProperties");
  params.set<UserObjectName>("fp_primary") = "fp_nitrogen";
  params.set<std::vector<UserObjectName>>("fp_secondary") = {"fp_tv_secondary"};
  params.set<bool>("allow_imperfect_jacobians") = true;
  _fe_problem->addUserObject("IdealRealGasMixtureFluidProperties", "fp_swapped_mix", params);
  const auto & fp_mix =
      _fe_problem->getUserObject<IdealRealGasMixtureFluidProperties>("fp_swapped_mix");

  const Real T = 400.0;
  const Real v = 1.1;
  const std::vector<Real> x = {0.3};

  Real mu, dmu_dT, dmu_dv;
  std::vector<Real> dmu_dx;
  fp_mix.mu_from_T_v(T, v, x, mu, dmu_dT, dmu_dv, dmu_dx);
  REL_TEST(fp_mix.mu_from_T_v(T, v, x), mu, REL_TOL_CONSISTENCY);

  Real k, dk_dT, dk_dv;
  std::vector<Real> dk_dx;
  fp_mix.k_from_T_v(T, v, x, k, dk_dT, dk_dv, dk_dx);
  REL_TEST(fp_mix.k_from_T_v(T, v, x), k, REL_TOL_CONSISTENCY);

  Moose::_throw_on_warning = original_throw_on_warning;
}

/**
 * Verify the mass fraction derivatives for a mixture of three components, where the secondary
 * mass fractions are independent and the primary mass fraction is 1 - sum of the others
 */
TEST_F(IdealRealGasMixtureFluidPropertiesTest, threeComponents)
{
  const bool original_throw_on_warning = Moose::_throw_on_warning;
  Moose::_throw_on_warning = false;

  {
    InputParameters params = _factory.getValidParams("IdealGasFluidProperties");
    params.set<Real>("gamma") = 1.67;
    params.set<Real>("molar_mass") = 0.039948;
    _fe_problem->addUserObject("IdealGasFluidProperties", "fp_argon", params);
  }
  InputParameters params = _factory.getValidParams("IdealRealGasMixtureFluidProperties");
  params.set<UserObjectName>("fp_primary") = "fp_steam";
  params.set<std::vector<UserObjectName>>("fp_secondary") = {"fp_nitrogen", "fp_argon"};
  params.set<bool>("allow_imperfect_jacobians") = true;
  _fe_problem->addUserObject("IdealRealGasMixtureFluidProperties", "fp_mix3", params);
  const auto & fp_mix3 = _fe_problem->getUserObject<IdealRealGasMixtureFluidProperties>("fp_mix3");

  const Real T = 400.0;
  const Real v = 1.1;
  const std::vector<Real> x = {0.3, 0.2};

  VAPOR_MIX_DERIV_TEST(fp_mix3.p_from_T_v, T, v, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(fp_mix3.e_from_T_v, T, v, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(fp_mix3.s_from_T_v, T, v, x, REL_TOL_DERIVATIVE);

  Moose::_throw_on_warning = original_throw_on_warning;
}

/**
 * Verify that the maximum mixture temperature can be set with the T_mix_max parameter
 */
TEST_F(IdealRealGasMixtureFluidPropertiesTest, maximumTemperatureParameter)
{
  InputParameters params = _factory.getValidParams("IdealRealGasMixtureFluidProperties");
  EXPECT_TRUE(params.have_parameter<Real>("T_mix_max"));
  EXPECT_EQ(params.get<Real>("T_mix_max"), 1300.0);
}

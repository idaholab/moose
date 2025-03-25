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

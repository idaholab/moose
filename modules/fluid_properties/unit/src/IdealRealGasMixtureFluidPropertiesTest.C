//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  Real T = 400.;
  Real p = 100000.;
  std::vector<Real> x = {0.9};

  // pure fluids
  Real rho_nitrogen = _fp_nitrogen->rho_from_p_T(p, T);
  Real rho_steam = _fp_steam->rho_from_p_T(p, T);
  REL_TEST(rho_nitrogen, 0.84228968026683726, REL_TOL_CONSISTENCY);
  REL_TEST(rho_steam, 0.61493738819320221, REL_TOL_CONSISTENCY);

  // mixture properties
  Real rho_mix = _fp_mix->rho_from_p_T(p, T, x);
  Real v_mix = _fp_mix->v_from_p_T(p, T, x);
  Real e_mix = _fp_mix->e_from_p_T(p, T, x);
  Real c_mix = _fp_mix->c_from_p_T(p, T, x);
  Real cp_mix = _fp_mix->cp_from_p_T(p, T, x);
  Real cv_mix = _fp_mix->cv_from_p_T(p, T, x);
  Real mu_mix = _fp_mix->mu_from_p_T(p, T, x);
  Real k_mix = _fp_mix->k_from_p_T(p, T, x);
  REL_TEST(rho_mix, 0.88183704292782583, REL_TOL_CONSISTENCY);
  REL_TEST(v_mix, 1.1339963636363655, REL_TOL_CONSISTENCY);
  REL_TEST(e_mix, 523068.96363636368, REL_TOL_CONSISTENCY);
  REL_TEST(c_mix, 418.49693534182865, REL_TOL_CONSISTENCY);
  REL_TEST(cp_mix, 1083.6715000000002, REL_TOL_CONSISTENCY);
  REL_TEST(cv_mix, 771.82250000000022, REL_TOL_CONSISTENCY);
  REL_TEST(mu_mix, 2.08926978862e-5, REL_TOL_CONSISTENCY);
  REL_TEST(k_mix, 0.0319250086968, REL_TOL_CONSISTENCY);

  // check inverse functions of (v,e)
  Real p_mix = _fp_mix->p_from_v_e(v_mix, e_mix, x);
  Real T_mix = _fp_mix->T_from_v_e(v_mix, e_mix, x);
  REL_TEST(p_mix, p, REL_TOL_CONSISTENCY);
  REL_TEST(T_mix, T, REL_TOL_CONSISTENCY);

  // check T(p,v) and its derivatives
  REL_TEST(T, _fp_mix->T_from_p_v(p, v_mix, x), REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->T_from_p_v, p, v_mix, x, REL_TOL_DERIVATIVE);

  // check e(p,rho) and its derivatives
  REL_TEST(e_mix, _fp_mix->e_from_p_rho(p, rho_mix, x), REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_p_rho, p, rho_mix, x, REL_TOL_DERIVATIVE);

  //////////////////////////////////////////////////////////////////////////////
  // note that the VAPOR_MIX_DERIV_TEST works for binary mixtures only (for now)
  //////////////////////////////////////////////////////////////////////////////

  // check analytical derivatives from (p,T)
  VAPOR_MIX_DERIV_TEST(_fp_mix->rho_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->v_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  // the following test fails since the pertubation in VAPOR_MIX_DERIV_TEST
  // is too small if v(p,T) is iterated within e_from_p_T
  // VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  Real e, de_dp, de_dT;
  std::vector<Real> de_dx;
  de_dx.resize(_fp_mix->getNumberOfSecondaryVapors());
  _fp_mix->e_from_p_T(p, T, x, e, de_dp, de_dT, de_dx);
  Real dp = p * 1.e-5;
  Real p1 = _fp_mix->e_from_p_T(p - dp, T, x);
  Real p2 = _fp_mix->e_from_p_T(p + dp, T, x);
  Real de_dp_num = (p2 - p1) / (2.0 * dp);
  REL_TEST(de_dp, de_dp_num, REL_TOL_DERIVATIVE);

  // check analytical derivatives from (v,e)
  VAPOR_MIX_DERIV_TEST(_fp_mix->p_from_v_e, v_mix, e_mix, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->T_from_v_e, v_mix, e_mix, x, REL_TOL_DERIVATIVE);

  // check analytical derivatives from (T,v)
  VAPOR_MIX_DERIV_TEST(_fp_mix->p_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_T_v, T, v_mix, x, REL_TOL_DERIVATIVE);

  // state-of-the-art mixture model comparison
  T = 360.;
  p = 100000.;
  x[0] = 0.5;
  rho_mix = _fp_mix->rho_from_p_T(p, T, x); // 0.7362935
  v_mix = _fp_mix->v_from_p_T(p, T, x);     // 1.3581540
  e_mix = _fp_mix->e_from_p_T(p, T, x);     // 1380586.9
  c_mix = _fp_mix->c_from_p_T(p, T, x);     // 428.1928
  cp_mix = _fp_mix->cp_from_p_T(p, T, x);   // 1479.7114
  cv_mix = _fp_mix->cv_from_p_T(p, T, x);   // 1090.5912
  // these quantities cannot be calculated with the state-of-the-art mixture model
  // mu_mix = _fp_mix->mu_from_p_T(p, T, x);
  // k_mix = _fp_mix->k_from_p_T(p, T, x);

  REL_TEST(rho_mix, 0.7362935, 0.115554);
  REL_TEST(v_mix, 1.3581540, 0.103585);
  REL_TEST(e_mix, 1380586.9, 0.023648);
  REL_TEST(c_mix, 428.1928, 0.017535);
  REL_TEST(cp_mix, 1479.7114, 0.146444);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cp_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  REL_TEST(cv_mix, 1090.5912, 0.183001);
  VAPOR_MIX_DERIV_TEST(_fp_mix->cv_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->mu_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
  VAPOR_MIX_DERIV_TEST(_fp_mix->k_from_p_T, p, T, x, REL_TOL_DERIVATIVE);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFluidPropsTest.h"
#include "DualReal.h"
#include "Moose.h"

TEST_F(ADFluidPropsTest, ad_basic)
{
  Real v = .7;
  Real e = 214000;

  Real p = 0;
  Real dpdv = 0;
  Real dpde = 0;
  _fp->p_from_v_e(v, e, p, dpdv, dpde);

  DNDerivativeType dvdx;
  DNDerivativeType dedx;
  Moose::derivInsert(dvdx, 0, 1);
  Moose::derivInsert(dvdx, 1, 2);
  Moose::derivInsert(dvdx, 2, 3);
  Moose::derivInsert(dedx, 0, 1);
  Moose::derivInsert(dedx, 1, 0);
  Moose::derivInsert(dedx, 2, 2);

  DualReal v_ad(v, dvdx);
  DualReal e_ad(e, dedx);
  auto p_ad = _fp->p_from_v_e(v_ad, e_ad);

  EXPECT_DOUBLE_EQ(p, p_ad.value());
  for (size_t i = 0; i < 3; i++)
    EXPECT_DOUBLE_EQ(dpdv * dvdx[i] + dpde * dedx[i], p_ad.derivatives()[i]);
}

TEST_F(ADFluidPropsTest, ad_two_phase)
{
  const Real p = 1e5;
  const Real T = 300;

  DNDerivativeType dpdU;
  Moose::derivInsert(dpdU, 0, 1);
  Moose::derivInsert(dpdU, 1, 2);
  Moose::derivInsert(dpdU, 2, 3);

  DNDerivativeType dTdU;
  Moose::derivInsert(dTdU, 0, 1);
  Moose::derivInsert(dTdU, 1, 0);
  Moose::derivInsert(dTdU, 2, 2);

  const DualReal p_ad(p, dpdU);
  const DualReal T_ad(T, dTdU);

  const auto & fp_2phase = buildTwoPhaseFluidProperties();
  const auto & fp_liquid = _fe_problem->getUserObject<SinglePhaseFluidProperties>(fp_2phase.getLiquidName());
  const auto & fp_vapor = _fe_problem->getUserObject<SinglePhaseFluidProperties>(fp_2phase.getVaporName());

  // Latent heat

  const Real h_lat = fp_2phase.h_lat(p, T);

  Real h_liquid = 0;
  Real dh_liquid_dp = 0;
  Real dh_liquid_dT = 0;
  fp_liquid.h_from_p_T(p, T, h_liquid, dh_liquid_dp, dh_liquid_dT);

  Real h_vapor = 0;
  Real dh_vapor_dp = 0;
  Real dh_vapor_dT = 0;
  fp_vapor.h_from_p_T(p, T, h_vapor, dh_vapor_dp, dh_vapor_dT);

  const Real dh_lat_dp = dh_vapor_dp - dh_liquid_dp;
  const Real dh_lat_dT = dh_vapor_dT - dh_liquid_dT;

  const DualReal h_lat_ad = fp_2phase.h_lat(p_ad, T_ad);

  EXPECT_DOUBLE_EQ(h_lat, h_lat_ad.value());
  for (size_t i = 0; i < 3; i++)
    EXPECT_DOUBLE_EQ(dh_lat_dp * dpdU[i] + dh_lat_dT * dTdU[i], h_lat_ad.derivatives()[i]);

  // Surface tension

  const Real sigma = fp_2phase.sigma_from_T(T);
  const Real dsigma_dT = fp_2phase.dsigma_dT_from_T(T);
  const DualReal sigma_ad = fp_2phase.sigma_from_T(T_ad);

  EXPECT_DOUBLE_EQ(sigma, sigma_ad.value());
  for (size_t i = 0; i < 3; i++)
    EXPECT_DOUBLE_EQ(dsigma_dT * dTdU[i], sigma_ad.derivatives()[i]);

  // Saturation temperature

  const Real T_sat = fp_2phase.T_sat(p);
  const Real dT_sat_dp = fp_2phase.dT_sat_dp(p);
  const DualReal T_sat_ad = fp_2phase.T_sat(p_ad);

  EXPECT_DOUBLE_EQ(T_sat, T_sat_ad.value());
  for (size_t i = 0; i < 3; i++)
    EXPECT_DOUBLE_EQ(dT_sat_dp * dpdU[i], T_sat_ad.derivatives()[i]);

  // Saturation pressure

  const Real p_sat = fp_2phase.p_sat(T);
  const Real dp_sat_dT = 1.0 / fp_2phase.dT_sat_dp(p_sat);
  const DualReal p_sat_ad = fp_2phase.p_sat(T_ad);

  EXPECT_DOUBLE_EQ(p_sat, p_sat_ad.value());
  for (size_t i = 0; i < 3; i++)
    EXPECT_DOUBLE_EQ(dp_sat_dT * dTdU[i], p_sat_ad.derivatives()[i]);
}

TEST_F(ADFluidPropsTest, error_imperfect_jacobian)
{
  DualReal v = .7;
  DualReal e = 214000;

  // This throws because g_from_v_e has no derivatives version implemented:
  EXPECT_THROW(_fp->g_from_v_e(v, e), std::runtime_error);

  // create fp with allow_imperfect_jacobians on - but warnings are errors still
  auto & fp = buildObj("fp2", true);
  EXPECT_THROW(fp.g_from_v_e(v, e), std::runtime_error);

  // missing derivs become warnings instead of errors
  bool wae = Moose::_warnings_are_errors;
  bool toe = Moose::_throw_on_error;
  Moose::_warnings_are_errors = false;
  Moose::_throw_on_error = false;
  EXPECT_NO_THROW(fp.g_from_v_e(v, e));
  Moose::_warnings_are_errors = wae;
  Moose::_throw_on_error = toe;
}

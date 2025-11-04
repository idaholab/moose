//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasMixtureFluidPropertiesTest.h"
#include "VaporMixtureFluidPropertiesUtils.h"
#include "FluidProperties.h"

TEST_F(IdealGasMixtureFluidPropertiesTest, test)
{
  Real T = 400;
  Real p = 100000;
  std::vector<Real> x = {0.7};

  const Real v = 1.38490747936373;
  const Real rho = 0.7220698962933115;
  const Real e = 1.995566878921797e+06;
  const Real s = 3129.64317643634;
  const Real c = 442.127817187648;
  const Real cp = 1187.64406714542;
  const Real cv = 841.417197304492;
  const Real mu = 1.863687329029527e-05;
  const Real k = 0.0304142253602899;

  REL_TEST(_fp_mix->p_from_v_e(v, e, x), p, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->T_from_v_e(v, e, x), T, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->v_from_p_T(p, T, x), v, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->rho_from_p_T(p, T, x), rho, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->e_from_p_T(p, T, x), e, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->e_from_p_rho(p, rho, x), e, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->s_from_p_T(p, T, x), s, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), c, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cp_from_p_T(p, T, x), cp, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cv_from_p_T(p, T, x), cv, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->mu_from_p_T(p, T, x), mu, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->k_from_p_T(p, T, x), k, REL_TOL_SAVED_VALUE);

  // Test sound speed against thermodynamics definition:
  //   c = sqrt((dp/drho)_s) = v sqrt(-(dp/dv)_s)
  // To compute (dp/dv)_s, we will use the following identity:
  //   (dp/dv)_s = 1 / [(dv/dp)_T - (dv/dT)_p (ds/dp)_T / (ds/dT)_p]
  const Real rel_pert = REL_PERTURBATION;
  const Real fpert1 = 1 + rel_pert;
  const Real fpert2 = 1 - rel_pert;
  const Real dvdp_T = (_fp_mix->v_from_p_T(p * fpert1, T, x) - _fp_mix->v_from_p_T(p * fpert2, T, x)) / (2 * rel_pert * p);
  const Real dvdT_p = (_fp_mix->v_from_p_T(p, T * fpert1, x) - _fp_mix->v_from_p_T(p, T * fpert2, x)) / (2 * rel_pert * T);
  const Real dsdp_T = (_fp_mix->s_from_p_T(p * fpert1, T, x) - _fp_mix->s_from_p_T(p * fpert2, T, x)) / (2 * rel_pert * p);
  const Real dsdT_p = (_fp_mix->s_from_p_T(p, T * fpert1, x) - _fp_mix->s_from_p_T(p, T * fpert2, x)) / (2 * rel_pert * T);
  const Real dpdv_s = 1.0 / (dvdp_T - dvdT_p * dsdp_T / dsdT_p);
  const Real c_gold = v * std::sqrt(-dpdv_s);
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), c_gold, REL_TOL_DERIVATIVE);

  const Real x_steam = 0.3;
  const Real x_nitrogen = 0.7;

  const Real M_steam = 0.01801488;
  const Real M_nitrogen = 0.028012734746133888;
  const Real M = 1.0 / (x_steam / M_steam + x_nitrogen / M_nitrogen);

  const Real v_gold = FluidProperties::_R * T / (M * p);
  REL_TEST(v, v_gold, REL_TOL_SAVED_VALUE);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  const Real e = 1.995566878921797e+06;
  const Real s = 3129.64317643634;
  const Real c = 372.143450750342;
  const Real cp = 1187.64406714542;
  const Real cv = 841.417197304492;
  const Real mu = 1.863687329029527e-05;
  const Real k = 0.0304142253602899;

  REL_TEST(_fp_mix->p_from_v_e(v, e, x), p, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->T_from_v_e(v, e, x), T, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->v_from_p_T(p, T, x), v, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->e_from_p_T(p, T, x), e, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->s_from_p_T(p, T, x), s, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), c, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cp_from_p_T(p, T, x), cp, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cv_from_p_T(p, T, x), cv, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->mu_from_p_T(p, T, x), mu, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->k_from_p_T(p, T, x), k, REL_TOL_SAVED_VALUE);

  const Real x_steam = 0.3;
  const Real x_nitrogen = 0.7;

  const Real M_steam = 0.01801488;
  const Real M_nitrogen = 0.028012734746133888;
  const Real M = 1.0 / (x_steam / M_steam + x_nitrogen / M_nitrogen);

  const Real v_gold = FluidProperties::_R * T / (M * p);
  REL_TEST(v, v_gold, REL_TOL_SAVED_VALUE);
}

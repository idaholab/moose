//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralVaporMixtureFluidPropertiesTest.h"
#include "VaporMixtureFluidPropertiesUtils.h"
#include <cstdlib>

TEST_F(GeneralVaporMixtureFluidPropertiesTest, test)
{
  const Real p = 1e5;
  const Real T = 400;
  const std::vector<Real> x = {0.4};

  const Real v_steam = _fp_steam->v_from_p_T(p, T);
  const Real e_steam = _fp_steam->e_from_p_T(p, T);
  const Real v_air = _fp_air->v_from_p_T(p, T);
  const Real e_air = _fp_air->e_from_p_T(p, T);

  const Real v = (1 - x[0]) * v_steam + x[0] * v_air;
  const Real e = (1 - x[0]) * e_steam + x[0] * e_air;
  const Real rho = 1.0 / v;

  REL_TEST(_fp_mix->rho_from_p_T(p, T, x), rho, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->rho_from_p_T, p, T, x, REL_TOL_DERIVATIVE);

  REL_TEST(_fp_mix->e_from_p_T(p, T, x), e, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->e_from_p_T, p, T, x, REL_TOL_DERIVATIVE);

  REL_TEST(_fp_mix->c_from_p_T(p, T, x), 465.938962340801, REL_TOL_SAVED_VALUE);

  REL_TEST(_fp_mix->cp_from_p_T(p, T, x), 1294.2012, REL_TOL_SAVED_VALUE);

  REL_TEST(_fp_mix->cv_from_p_T(p, T, x), 911.058, REL_TOL_SAVED_VALUE);

  REL_TEST(_fp_mix->mu_from_p_T(p, T, x), 7.12234831455e-4, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->k_from_p_T(p, T, x), 0.431661904928, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->p_from_v_e(v, e, x), p, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->p_from_v_e, v, e, x, REL_TOL_DERIVATIVE * 2);

  REL_TEST(_fp_mix->T_from_v_e(v, e, x), T, REL_TOL_CONSISTENCY);
  VAPOR_MIX_DERIV_TEST(_fp_mix->T_from_v_e, v, e, x, REL_TOL_DERIVATIVE * 3e1);
}

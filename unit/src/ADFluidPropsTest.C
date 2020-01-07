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

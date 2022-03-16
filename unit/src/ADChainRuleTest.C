//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ADChainRuleTest.h"

TEST_F(ADChainRuleTest, ChainedReal)
{
  Real x = 5;
  ChainedReal var_x(x, 1);

  // Both the value and the derivative should be correct:
  EXPECT_NEAR(f<Real>(x), f<ChainedReal>(var_x).value(), 1e-12);
  EXPECT_NEAR(df_dx<Real>(x), f<ChainedReal>(var_x).derivatives(), 1e-12);
}

TEST_F(ADChainRuleTest, ChainedADReal)
{
  ADReal x = 5;
  Moose::derivInsert(x.derivatives(), 0, 1.7);
  Moose::derivInsert(x.derivatives(), 2, -9);
  ChainedADReal var_x(x, 1);

  // Both the value and the derivatives should be correct:
  EXPECT_NEAR(f<ADReal>(x).value(), f<ChainedADReal>(var_x).value().value(), 1e-12);
  EXPECT_NEAR(
      f<ADReal>(x).derivatives()[0], f<ChainedADReal>(var_x).value().derivatives()[0], 1e-12);
  EXPECT_NEAR(
      f<ADReal>(x).derivatives()[2], f<ChainedADReal>(var_x).value().derivatives()[2], 1e-12);

  EXPECT_NEAR(df_dx<ADReal>(x).value(), f<ChainedADReal>(var_x).derivatives().value(), 1e-12);
  EXPECT_NEAR(df_dx<ADReal>(x).derivatives()[0],
              f<ChainedADReal>(var_x).derivatives().derivatives()[0],
              1e-12);
  EXPECT_NEAR(df_dx<ADReal>(x).derivatives()[2],
              f<ChainedADReal>(var_x).derivatives().derivatives()[2],
              1e-12);
}

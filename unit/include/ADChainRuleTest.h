//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

#include "ADReal.h"
#include "ChainedReal.h"
#include "ChainedADReal.h"

class ADChainRuleTest : public ::testing::Test
{
protected:
  template <typename T>
  T f(const T & x)
  {
    using std::exp, std::pow, std::sin, std::cos;

    return 2 * x + 3 + exp(x) - pow(6 * x, 2) + sin(x - 3) * cos(-x);
  }

  template <typename T>
  T df_dx(const T & x)
  {
    using std::exp, std::pow, std::sin, std::cos;

    return 2 + exp(x) - 72 * x + cos(x - 3) * cos(-x) + sin(x - 3) * sin(-x);
  }
};

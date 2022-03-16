//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    return 2 * x + 3 + std::exp(x) - std::pow(6 * x, 2) + std::sin(x - 3) * std::cos(-x);
  }

  template <typename T>
  T df_dx(const T & x)
  {
    return 2 + std::exp(x) - 72 * x + std::cos(x - 3) * std::cos(-x) +
           std::sin(x - 3) * std::sin(-x);
  }
};

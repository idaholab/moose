//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MathUtils.h"

TEST(MathUtilsTest, pow)
{
  ASSERT_DOUBLE_EQ(MathUtils::pow(1.2345, 73), std::pow(1.2345, 73));
  ASSERT_DOUBLE_EQ(MathUtils::pow(-0.99542, 58), std::pow(-0.99542, 58));
}

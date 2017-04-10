/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "gtest/gtest.h"

#include "MathUtils.h"

TEST(MathUtilsTest, pow)
{
  ASSERT_DOUBLE_EQ(MathUtils::pow(1.2345, 73), std::pow(1.2345, 73));
  ASSERT_DOUBLE_EQ(MathUtils::pow(-0.99542, 58), std::pow(-0.99542, 58));
}

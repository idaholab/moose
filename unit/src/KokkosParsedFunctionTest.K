//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedFunctionTest.h"

#include "MooseUnitUtils.h"

TEST_F(KokkosParsedFunctionTest, ParserTest)
{
  for (const auto & [expression, expected] : valid_cases)
  {
    KokkosParsedFunctionTestObject object(constants, expression);
    Kokkos::parallel_for(1, object);
    EXPECT_NEAR(object.result(), expected.value, 1e-8);
  }

  for (const auto & it : invalid_cases)
    EXPECT_MOOSEERROR_MSG(KokkosParsedFunctionTestObject object(constants, it.first),
                          "Kokkos parsed function error: expression parsing failed.");

  for (const auto & it : semantic_error_cases)
    EXPECT_MOOSEERROR_MSG_CONTAINS(KokkosParsedFunctionTestObject object(constants, it.first),
                                   it.second);
}

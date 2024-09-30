//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantFunctionTest.h"
#include "FunctionTestUtils.h"

TEST_F(ConstantFunctionTest, testTimeIntegral)
{
  // build function
  const std::string obj_name = "testedfunction";
  InputParameters params = _factory.getValidParams("ConstantFunction");
  params.set<Real>("value") = 3.0;
  _fe_problem->addFunction("ConstantFunction", obj_name, params);
  const auto & f = _fe_problem->getFunction(obj_name);

  FunctionTestUtils::testTimeIntegral(f, 5.0, 20.0, Point(1.0, 2.0, 3.0), 1000, 1e-6);
}

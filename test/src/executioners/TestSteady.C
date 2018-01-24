//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSteady.h"
#include "MooseTestAppTypes.h"

template <>
InputParameters
validParams<TestSteady>()
{
  InputParameters params = validParams<Steady>();

  // Add control for the type of test to perform
  MooseEnum test_type("Exception addAttributeReporter");
  params.addParam<MooseEnum>(
      "test_type", test_type, "The type of test that this object should perform");
  return params;
}

TestSteady::TestSteady(const InputParameters & parameters)
  : Steady(parameters),
    _test_type(getParam<MooseEnum>("test_type")),
    _some_value_that_needs_to_be_reported(12345)
{
  if (_test_type == "addAttributeReporter")
    addAttributeReporter("luggage_combo", _some_value_that_needs_to_be_reported);
}

TestSteady::~TestSteady() {}

void
TestSteady::execute()
{
  Steady::execute();
}

void
TestSteady::postSolve()
{
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
}

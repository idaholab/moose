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
#include "MooseApp.h"
#include "ActionWarehouse.h"

registerMooseObject("MooseTestApp", TestSteady);

InputParameters
TestSteady::validParams()
{
  InputParameters params = Steady::validParams();

  // Add control for the type of test to perform
  MooseEnum test_type("Exception addAttributeReporter");
  params.addParam<MooseEnum>(
      "test_type", test_type, "The type of test that this object should perform");
  return params;
}

TestSteady::TestSteady(const InputParameters & parameters)
  : Steady(parameters), _test_type(getParam<MooseEnum>("test_type"))
{
  if (_test_type == "addAttributeReporter")
    _some_value_that_needs_to_be_reported = &addAttributeReporter("luggage_combo", 0);
}

TestSteady::~TestSteady() {}

void
TestSteady::preProblemInit()
{
  _console << "TestSteady::preProblemInit() is called while executing "
           << _app.actionWarehouse().getCurrentTaskName() << " task." << std::endl;
}

void
TestSteady::preExecute()
{
  if (_test_type == "addAttributeReporter")
    *_some_value_that_needs_to_be_reported = 12345;
}

void
TestSteady::postSolve()
{
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
  _problem.execute(EXEC_JUST_GO);
}

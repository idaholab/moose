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
#include "TestSteady.h"

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
  _problem.execute("JUST_GO");
  _problem.execute("JUST_GO");
  _problem.execute("JUST_GO");
  _problem.execute("JUST_GO");
}

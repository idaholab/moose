//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LateRestartLoadTest.h"

registerMooseObject("MooseTestApp", LateRestartLoadTest);

InputParameters
LateRestartLoadTest::validParams()
{
  auto params = GeneralUserObject::validParams();

  MooseEnum test_methods("store load load_no_restart");
  params.addRequiredParam<MooseEnum>("test_method", test_methods, "Which method to test");

  return params;
}

LateRestartLoadTest::LateRestartLoadTest(const InputParameters & params)
  : GeneralUserObject(params),
    _test_method(getParam<MooseEnum>("test_method")),
    _value(1337),
    _restartable_value(
        _test_method == "store" ? &declareRestartableData<unsigned int>("value", _value) : nullptr)
{
}

void
LateRestartLoadTest::initialSetup()
{
  if (_test_method != "store")
  {
    const auto has_value = hasRestartableData<unsigned int>("value");
    if (has_value != (_test_method == "load"))
      mooseError("Has value check failed, has_value=", has_value);

    const auto value = getRestartableData<unsigned int>("value");
    if (_test_method == "load" && value != _value)
      mooseError("Failed to load value, value=", value);
    if (_test_method == "load_no_restart" && value != 0)
      mooseError("Failed to load default value, value=", value);
  }
}

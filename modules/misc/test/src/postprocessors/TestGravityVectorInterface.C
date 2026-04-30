//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestGravityVectorInterface.h"

registerMooseObject("MiscTestApp", TestGravityVectorInterface);

InputParameters
TestGravityVectorInterface::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += GravityVectorInterface::validParams();
  MooseEnum test_value("magnitude x y z dir_x dir_y dir_z");
  params.addRequiredParam<MooseEnum>("test_value", test_value, "Test value to get");
  return params;
}

TestGravityVectorInterface::TestGravityVectorInterface(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), GravityVectorInterface(this)
{
}

PostprocessorValue
TestGravityVectorInterface::getValue() const
{
  const auto test_value = getParam<MooseEnum>("test_value");
  if (test_value == "magnitude")
    return gravityMagnitude();
  else if (test_value == "x")
    return gravityVector()(0);
  else if (test_value == "y")
    return gravityVector()(1);
  else if (test_value == "z")
    return gravityVector()(2);
  else if (test_value == "dir_x")
    return gravityDirection()(0);
  else if (test_value == "dir_y")
    return gravityDirection()(1);
  else if (test_value == "dir_z")
    return gravityDirection()(2);
  else
    mooseError("Invalid value");
}

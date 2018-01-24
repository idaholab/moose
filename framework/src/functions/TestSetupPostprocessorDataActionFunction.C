//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSetupPostprocessorDataActionFunction.h"

template <>
InputParameters
validParams<TestSetupPostprocessorDataActionFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "A postprocessor to test against");
  return params;
}

TestSetupPostprocessorDataActionFunction::TestSetupPostprocessorDataActionFunction(
    const InputParameters & parameters)
  : Function(parameters)
{
  if (hasPostprocessor("postprocessor"))
    mooseError("TestSetupPostprocessorDataActionFunction pass");
  else
    mooseError("TestSetupPostprocessorDataActionFunction fail");
}

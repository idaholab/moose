//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NullExecutor.h"

registerMooseObject("MooseApp", NullExecutor);

InputParameters
NullExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addClassDescription(
      "Dummy executor that does nothing. Useful for testing among other things.");
  return params;
}

NullExecutor::NullExecutor(const InputParameters & parameters) : Executor(parameters) {}

Executor::Result
NullExecutor::run()
{
  Result & result = newResult();
  return result;
}

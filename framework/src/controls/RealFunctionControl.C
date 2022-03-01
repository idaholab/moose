//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RealFunctionControl.h"
#include "Function.h"

registerMooseObject("MooseApp", RealFunctionControl);

InputParameters
RealFunctionControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription(
      "Sets the value of a 'Real' input parameters to the value of a provided function.");
  params.addRequiredParam<FunctionName>(
      "function", "The function to use for controlling the specified parameter.");
  params.addRequiredParam<std::string>(
      "parameter",
      "The input parameter(s) to control. Specify a single parameter name and all "
      "parameters in all objects matching the name will be updated");
  return params;
}

RealFunctionControl::RealFunctionControl(const InputParameters & parameters)
  : Control(parameters), _function(getFunction("function"))
{
}

void
RealFunctionControl::execute()
{
  Real value = _function.value(_t);
  setControllableValue<Real>("parameter", value);
}

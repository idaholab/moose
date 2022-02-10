//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeFunctionComponentControl.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", TimeFunctionComponentControl);

InputParameters
TimeFunctionComponentControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("component",
                                       "The name of the component we will be controlling.");
  params.addRequiredParam<std::string>(
      "parameter", "The name of the parameter in the component we will be controlling");
  params.addRequiredParam<FunctionName>("function",
                                        "The name of the function prescribing the value.");
  return params;
}

TimeFunctionComponentControl::TimeFunctionComponentControl(const InputParameters & parameters)
  : THMControl(parameters),
    _component_name(getParam<std::string>("component")),
    _param_name(getParam<std::string>("parameter")),
    _ctrl_param_name("component", _component_name, _param_name),
    _function(getFunction("function"))
{
}

void
TimeFunctionComponentControl::execute()
{
  Real value = _function.value(_t, Point());
  setControllableValueByName<Real>(_ctrl_param_name, value);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetFunctionValueControl.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", GetFunctionValueControl);

InputParameters
GetFunctionValueControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addClassDescription("Sets a ControlData named 'value' with the value of a function");
  params.addRequiredParam<FunctionName>("function",
                                        "The name of the function prescribing a value.");
  return params;
}

GetFunctionValueControl::GetFunctionValueControl(const InputParameters & parameters)
  : THMControl(parameters),
    _value(declareComponentControlData<Real>("value")),
    _function(getFunction("function"))
{
}

void
GetFunctionValueControl::execute()
{
  _value = _function.value(_t, Point());
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetFunctionValueChainControl.h"
#include "Function.h"

registerMooseObject("MooseApp", GetFunctionValueChainControl);

InputParameters
GetFunctionValueChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();
  params.addClassDescription("Creates a control data and populates it by evaluating a Function.");
  params.addRequiredParam<FunctionName>("function", "Function to be evaluated");
  params.addParam<Point>("point", Point(), "Spatial point at which to evaluate the function");
  return params;
}

GetFunctionValueChainControl::GetFunctionValueChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _value(declareChainControlData<Real>("value")),
    _function(getFunction("function")),
    _point(getParam<Point>("point"))
{
}

void
GetFunctionValueChainControl::execute()
{
  _value = _function.value(_t, _point);
}

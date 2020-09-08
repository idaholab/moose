//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDerivativeAux.h"
#include "Function.h"

registerMooseObject("MooseTestApp", FunctionDerivativeAux);

InputParameters
FunctionDerivativeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  MooseEnum component_enum("x y z t");
  params.addRequiredParam<MooseEnum>(
      "component", component_enum, "What component to take the derivative with respect to.");
  return params;
}

FunctionDerivativeAux::FunctionDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _func(getFunction("function")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
FunctionDerivativeAux::computeValue()
{
  if (_component < 3)
  {
    if (isNodal())
      return _func.gradient(_t, *_current_node)(_component);
    else
      return _func.gradient(_t, _q_point[_qp])(_component);
  }
  else
  {
    if (isNodal())
      return _func.timeDerivative(_t, *_current_node);
    else
      return _func.timeDerivative(_t, _q_point[_qp]);
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionScalarAux.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionScalarAux);

InputParameters
FunctionScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addClassDescription("Sets a value of a scalar variable based on a function.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "function", "The functions to set the scalar variable components.");

  return params;
}

FunctionScalarAux::FunctionScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters)
{
  std::vector<FunctionName> funcs = getParam<std::vector<FunctionName>>("function");
  if (funcs.size() != _var.order())
    mooseError("number of functions is not equal to the number of scalar variable components");

  for (const auto & func : funcs)
    _functions.push_back(&getFunctionByName(func));
}

Real
FunctionScalarAux::computeValue()
{
  return _functions[_i]->value(_t, _point_zero);
}

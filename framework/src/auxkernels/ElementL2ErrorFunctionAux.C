//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementL2ErrorFunctionAux.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementL2ErrorFunctionAux);

InputParameters
ElementL2ErrorFunctionAux::validParams()
{
  InputParameters params = ElementLpNormAux::validParams();
  params.addClassDescription("A class for computing the element-wise L^2 (Euclidean) error between "
                             "a function and a coupled variable.");
  params.addRequiredParam<FunctionName>("function", "Function representing the exact solution");
  return params;
}

ElementL2ErrorFunctionAux::ElementL2ErrorFunctionAux(const InputParameters & parameters)
  : ElementLpNormAux(parameters), _func(getFunction("function"))
{
}

Real
ElementL2ErrorFunctionAux::computeValue()
{
  return _func.value(_t, _q_point[_qp]) - _coupled_var[_qp];
}

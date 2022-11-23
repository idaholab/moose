//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorFunctionNeumannBC.h"
#include "Function.h"
#include "MooseTypes.h"

registerMooseObject("MooseApp", ADVectorFunctionNeumannBC);

InputParameters
ADVectorFunctionNeumannBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addParam<FunctionName>("function_x", 0, "The function for the x component");
  params.addParam<FunctionName>("function_y", 0, "The function for the y component");
  params.addParam<FunctionName>("function_z", 0, "The function for the z component");
  params.addClassDescription(
      "Imposes the integrated boundary condition "
      "$\\frac{\\partial \\vec{u}}{\\partial n} = \\vec{h}$, "
      "where $\\vec{h}$ is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

ADVectorFunctionNeumannBC::ADVectorFunctionNeumannBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z"))
{
}

ADReal
ADVectorFunctionNeumannBC::computeQpResidual()
{
  ADRealVectorValue func_u = {_function_x.value(_t, _q_point[_qp]),
                              _function_y.value(_t, _q_point[_qp]),
                              _function_z.value(_t, _q_point[_qp])};

  return -_test[_i][_qp] * func_u;
}

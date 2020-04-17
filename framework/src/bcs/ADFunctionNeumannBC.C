//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionNeumannBC.h"

#include "Function.h"

registerMooseObject("MooseApp", ADFunctionNeumannBC);

InputParameters
ADFunctionNeumannBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("function", "The function.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h(t,\\vec{x})$, "
                             "where $h$ is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

ADFunctionNeumannBC::ADFunctionNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _func(getFunction("function"))
{
}

ADReal
ADFunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _func.value(_t, _q_point[_qp]);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionNeumannBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionNeumannBC);

InputParameters
FunctionNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("function", "The function.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h(t,\\vec{x})$, "
                             "where $h$ is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

FunctionNeumannBC::FunctionNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _func(getFunction("function"))
{
}

Real
FunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _func.value(_t, _q_point[_qp]);
}

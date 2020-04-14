//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ADFunctionPenaltyDirichletBC);

InputParameters
ADFunctionPenaltyDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Enforces a (possibly) time and space-dependent MOOSE Function Dirichlet boundary condition "
      "in a weak sense by penalizing differences between the current "
      "solution and the Dirichlet data.");
  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addRequiredParam<FunctionName>("function", "Forcing function");

  return params;
}

ADFunctionPenaltyDirichletBC::ADFunctionPenaltyDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _func(getFunction("function")), _p(getParam<Real>("penalty"))
{
}

ADReal
ADFunctionPenaltyDirichletBC::computeQpResidual()
{
  return _p * _test[_i][_qp] * (-_func.value(_t, _q_point[_qp]) + _u[_qp]);
}

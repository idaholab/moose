//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyDirichletNodalKernel.h"

registerMooseObject("MooseApp", PenaltyDirichletNodalKernel);

InputParameters
PenaltyDirichletNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Enforces a Dirichlet boundary condition "
                             "in a weak sense by penalizing differences between the current "
                             "solution and the Dirichlet value on nodesets.");

  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addParam<Real>("value", 0.0, "Boundary value of the variable");
  params.declareControllable("value");

  return params;
}

PenaltyDirichletNodalKernel::PenaltyDirichletNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _p(getParam<Real>("penalty")), _v(getParam<Real>("value"))
{
}

Real
PenaltyDirichletNodalKernel::computeQpResidual()
{
  return _p * (-_v + _u[_qp]);
}

Real
PenaltyDirichletNodalKernel::computeQpJacobian()
{
  return _p;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyDirichletBC.h"

registerMooseObject("MooseApp", PenaltyDirichletBC);

InputParameters
PenaltyDirichletBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addParam<Real>("value", 0.0, "Boundary value of the variable");
  params.declareControllable("value");
  params.addClassDescription("Enforces a Dirichlet boundary condition "
                             "in a weak sense by penalizing differences between the current "
                             "solution and the Dirichlet data.");
  return params;
}

PenaltyDirichletBC::PenaltyDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _p(getParam<Real>("penalty")), _v(getParam<Real>("value"))
{
}

Real
PenaltyDirichletBC::computeQpResidual()
{
  return _p * _test[_i][_qp] * (-_v + _u[_qp]);
}

Real
PenaltyDirichletBC::computeQpJacobian()
{
  return _p * _phi[_j][_qp] * _test[_i][_qp];
}

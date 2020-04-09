//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyDirichletOldValuePD.h"

registerMooseObject("PeridynamicsApp", PenaltyDirichletOldValuePD);

InputParameters
PenaltyDirichletOldValuePD::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Enforces a Dirichlet boundary condition "
                             "in a weak sense by penalizing differences between the current "
                             "solution and the old solution for transient problems.");

  params.addRequiredParam<Real>("penalty", "Penalty scalar");

  return params;
}

PenaltyDirichletOldValuePD::PenaltyDirichletOldValuePD(const InputParameters & parameters)
  : NodalKernel(parameters), _p(getParam<Real>("penalty")), _u_old(_var.dofValuesOld())
{
}

Real
PenaltyDirichletOldValuePD::computeQpResidual()
{
  return _p * (-_u_old[_qp] + _u[_qp]);
}

Real
PenaltyDirichletOldValuePD::computeQpJacobian()
{
  return _p;
}

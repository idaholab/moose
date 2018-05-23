//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RateDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", RateDirichletBC);

template <>
InputParameters
validParams<RateDirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<Real>("rate", "Value of the rate.");
  params.declareControllable("rate");
  params.addClassDescription(
      "Imposes the incremental boundary condition $du = g * dt$"
      "where $g$ is a controllable constant.");
  return params;
}

RateDirichletBC::RateDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _rate(getParam<Real>("rate")),
    _u_old(_var.dofValuesOld())
{
}

Real
RateDirichletBC::computeQpResidual()
{
  return _u[_qp] - (_u_old[_qp] + _rate * _dt);
}

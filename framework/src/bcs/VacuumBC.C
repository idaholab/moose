//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VacuumBC.h"

template <>
InputParameters
validParams<VacuumBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("alpha", 1, "No idea.");
  return params;
}

VacuumBC::VacuumBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _alpha(getParam<Real>("alpha"))
{
}

Real
VacuumBC::computeQpResidual()
{
  return _test[_i][_qp] * _alpha * _u[_qp] / 2.;
}

Real
VacuumBC::computeQpJacobian()
{
  return _test[_i][_qp] * _alpha * _phi[_j][_qp] / 2.;
}

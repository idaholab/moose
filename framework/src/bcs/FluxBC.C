//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxBC.h"

InputParameters
FluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  return params;
}

FluxBC::FluxBC(const InputParameters & params) : IntegratedBC(params) {}

Real
FluxBC::computeQpResidual()
{
  return -computeQpFluxResidual() * _normals[_qp] * _test[_i][_qp];
}

Real
FluxBC::computeQpJacobian()
{
  return -computeQpFluxJacobian() * _normals[_qp] * _test[_i][_qp];
}

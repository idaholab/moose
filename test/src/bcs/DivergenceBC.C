//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DivergenceBC.h"

registerMooseObject("MooseTestApp", DivergenceBC);

InputParameters
DivergenceBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  return params;
}

DivergenceBC::DivergenceBC(const InputParameters & parameters) : IntegratedBC(parameters) {}

Real
DivergenceBC::computeQpResidual()
{
  return -_test[_i][_qp] * (_grad_u[_qp] * _normals[_qp]);
}

Real
DivergenceBC::computeQpJacobian()
{
  return -_test[_i][_qp] * (_grad_phi[_j][_qp] * _normals[_qp]);
}

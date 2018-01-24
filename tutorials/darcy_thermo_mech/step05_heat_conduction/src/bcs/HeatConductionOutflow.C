//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionOutflow.h"

template <>
InputParameters
validParams<HeatConductionOutflow>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

HeatConductionOutflow::HeatConductionOutflow(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // IntegratedBCs can retrieve material properties!
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{
}

Real
HeatConductionOutflow::computeQpResidual()
{
  return -_test[_i][_qp] * _thermal_conductivity[_qp] * _grad_u[_qp] * _normals[_qp];
}

Real
HeatConductionOutflow::computeQpJacobian()
{
  // Derivative of the residual with respect to "u"
  return -_test[_i][_qp] * _thermal_conductivity[_qp] * _grad_phi[_j][_qp] * _normals[_qp];
}

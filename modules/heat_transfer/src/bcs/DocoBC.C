//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DocoBC.h"

registerMooseObject("HeatTransferApp", DocoBC);

InputParameters
DocoBC::validParams()
{
  InputParameters params = FluxBC::validParams();

  return params;
}

DocoBC::DocoBC(const InputParameters & parameters)
  : FluxBC(parameters), _k(getMaterialProperty<Real>("thermal_conductivity"))
{
}

DocoBC::~DocoBC() {}

RealGradient
DocoBC::computeQpFluxResidual()
{
  return -_k[_qp] * _grad_u[_qp];
}

RealGradient
DocoBC::computeQpFluxJacobian()
{
  return -_k[_qp] * _grad_phi[_j][_qp];
}

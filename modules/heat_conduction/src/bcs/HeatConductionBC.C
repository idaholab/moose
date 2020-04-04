//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionBC.h"

registerMooseObject("HeatConductionApp", HeatConductionBC);

InputParameters
HeatConductionBC::validParams()
{
  InputParameters params = FluxBC::validParams();

  return params;
}

HeatConductionBC::HeatConductionBC(const InputParameters & parameters)
  : FluxBC(parameters), _k(getMaterialProperty<Real>("thermal_conductivity"))
{
}

HeatConductionBC::~HeatConductionBC() {}

RealGradient
HeatConductionBC::computeQpFluxResidual()
{
  return -_k[_qp] * _grad_u[_qp];
}

RealGradient
HeatConductionBC::computeQpFluxJacobian()
{
  return -_k[_qp] * _grad_phi[_j][_qp];
}

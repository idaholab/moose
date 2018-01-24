/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatConductionBC.h"

template <>
InputParameters
validParams<HeatConductionBC>()
{
  InputParameters params = validParams<FluxBC>();

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

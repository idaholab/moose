//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EnergyFreeBC.h"

registerMooseObject("NavierStokesApp", EnergyFreeBC);

InputParameters
EnergyFreeBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("rho_u", "x-component of momentum");
  params.addCoupledVar("rho_v", "y-component of momentum");
  params.addCoupledVar("rho_w", "z-component of momentum");
  params.addRequiredCoupledVar("enthalpy", "Enthalpy");
  params.addClassDescription(
      "Implements free advective flow boundary conditions for the energy equation.");
  return params;
}

EnergyFreeBC::EnergyFreeBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _enthalpy(coupledValue("enthalpy")),
    _rho_u(coupledValue("rho_u")),
    _rho_v(isCoupled("rho_v") ? coupledValue("rho_v") : _zero),
    _rho_w(isCoupled("rho_w") ? coupledValue("rho_w") : _zero)
{
}

Real
EnergyFreeBC::computeQpResidual()
{
  // (rho u) * H * n
  RealVectorValue rho_u_vec(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  return rho_u_vec * _enthalpy[_qp] * _normals[_qp] * _test[_i][_qp];
}

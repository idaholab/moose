//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedPower.h"

registerMooseObject("NavierStokesApp", DistributedPower);

InputParameters
DistributedPower::validParams()
{
  InputParameters params = Kernel::validParams();

  // The acceleration vector.
  params.addParam<RealVectorValue>(
      "acceleration",
      RealVectorValue(0, 0, 0),
      "The acceleration components for an applied distributed force in an element.");

  // Momentum components.
  params.addRequiredCoupledVar("rho_u", "x-component of the momentum vector");
  params.addCoupledVar("rho_v", "y-component of the momentum vector");
  params.addCoupledVar("rho_w", "z-component of the momentum vector");

  params.addClassDescription(
      "Implements the power term of a specified force in the Navier Stokes energy equation.");

  return params;
}

DistributedPower::DistributedPower(const InputParameters & parameters)
  : Kernel(parameters),
    // acceleration vector
    _acceleration(getParam<RealVectorValue>("acceleration")),

    // momentum components
    _rhou_var_number(coupled("rho_u")),
    _rhov_var_number(isCoupled("rho_v") ? coupled("rho_v") : libMesh::invalid_uint),
    _rhow_var_number(isCoupled("rho_w") ? coupled("rho_w") : libMesh::invalid_uint),
    _rho_u(coupledValue("rho_u")),
    _rho_v(isCoupled("rho_v") ? coupledValue("rho_v") : _zero),
    _rho_w(isCoupled("rho_w") ? coupledValue("rho_w") : _zero)
{
}

Real
DistributedPower::computeQpResidual()
{
  RealVectorValue rhou_vec(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  // -rhou dot acceleration
  return -rhou_vec * _acceleration * _test[_i][_qp];
}

Real
DistributedPower::computeQpJacobian()
{
  return 0.;
}

Real
DistributedPower::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhou_var_number)
    return -_phi[_j][_qp] * _acceleration(0) * _test[_i][_qp];
  if (jvar == _rhov_var_number)
    return -_phi[_j][_qp] * _acceleration(1) * _test[_i][_qp];
  if (jvar == _rhow_var_number)
    return -_phi[_j][_qp] * _acceleration(2) * _test[_i][_qp];

  return 0;
}

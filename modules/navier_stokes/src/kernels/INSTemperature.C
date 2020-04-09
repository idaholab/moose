//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSTemperature.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSTemperature);

InputParameters
INSTemperature::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the residual and Jacobian contributions for the "
                             "incompressible Navier-Stokes temperature (energy) equation.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  // Optional parameters
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");
  params.addParam<MaterialPropertyName>("k_name", "k", "thermal conductivity name");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "specific heat name");

  return params;
}

INSTemperature::INSTemperature(const InputParameters & parameters)
  : Kernel(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),

    // Material Properties
    _rho(getMaterialProperty<Real>("rho_name")),
    _k(getMaterialProperty<Real>("k_name")),
    _cp(getMaterialProperty<Real>("cp_name"))
{
}

Real
INSTemperature::computeQpResidual()
{
  // The convection part, rho * cp u.grad(T) * v.
  // Note: _u is the temperature variable, _grad_u is its gradient.
  Real convective_part = _rho[_qp] * _cp[_qp] *
                         (_u_vel[_qp] * _grad_u[_qp](0) + _v_vel[_qp] * _grad_u[_qp](1) +
                          _w_vel[_qp] * _grad_u[_qp](2)) *
                         _test[_i][_qp];

  // Thermal conduction part, k * grad(T) * grad(v)
  Real conduction_part = _k[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];

  return convective_part + conduction_part;
}

Real
INSTemperature::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real convective_part = _rho[_qp] * _cp[_qp] * (U * _grad_phi[_j][_qp]) * _test[_i][_qp];
  Real conduction_part = _k[_qp] * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);

  return convective_part + conduction_part;
}

Real
INSTemperature::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    Real convective_part = _rho[_qp] * _cp[_qp] * _phi[_j][_qp] * _grad_u[_qp](0) * _test[_i][_qp];
    return convective_part;
  }

  else if (jvar == _v_vel_var_number)
  {
    Real convective_part = _rho[_qp] * _cp[_qp] * _phi[_j][_qp] * _grad_u[_qp](1) * _test[_i][_qp];
    return convective_part;
  }

  else if (jvar == _w_vel_var_number)
  {
    Real convective_part = _rho[_qp] * _cp[_qp] * _phi[_j][_qp] * _grad_u[_qp](2) * _test[_i][_qp];
    return convective_part;
  }
  else
    return 0;
}

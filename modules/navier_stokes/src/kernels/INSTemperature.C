/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSTemperature.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSTemperature>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("This class computes the residual and Jacobian contributions for the "
                             "incompressible Navier-Stokes temperature (energy) equation.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  // Required parameters
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<Real>("k", "thermal conductivity");
  params.addRequiredParam<Real>("cp", "specific heat");

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

    // Required parameters
    _rho(getParam<Real>("rho")),
    _k(getParam<Real>("k")),
    _cp(getParam<Real>("cp"))
{
}

Real
INSTemperature::computeQpResidual()
{
  // The convection part, rho * cp u.grad(T) * v.
  // Note: _u is the temperature variable, _grad_u is its gradient.
  Real convective_part = _rho * _cp *
                         (_u_vel[_qp] * _grad_u[_qp](0) + _v_vel[_qp] * _grad_u[_qp](1) +
                          _w_vel[_qp] * _grad_u[_qp](2)) *
                         _test[_i][_qp];

  // Thermal conduction part, k * grad(T) * grad(v)
  Real conduction_part = _k * _grad_u[_qp] * _grad_test[_i][_qp];

  return convective_part + conduction_part;
}

Real
INSTemperature::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real convective_part = _rho * _cp * (U * _grad_phi[_j][_qp]) * _test[_i][_qp];
  Real conduction_part = _k * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);

  return convective_part + conduction_part;
}

Real
INSTemperature::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](0) * _test[_i][_qp];
    return convective_part;
  }

  else if (jvar == _v_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](1) * _test[_i][_qp];
    return convective_part;
  }

  else if (jvar == _w_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](2) * _test[_i][_qp];
    return convective_part;
  }
  else
    return 0;
}

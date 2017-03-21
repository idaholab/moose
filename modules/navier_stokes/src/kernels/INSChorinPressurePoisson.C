/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSChorinPressurePoisson.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSChorinPressurePoisson>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("This class computes the pressure Poisson solve which is part of the "
                             "'split' scheme used for solving the incompressible Navier-Stokes "
                             "equations.");
  // Coupled variables
  params.addRequiredCoupledVar("u_star", "star x-velocity");
  params.addCoupledVar("v_star", "star y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w_star", "star z-velocity"); // only required in 3D

  // Required parameters
  params.addRequiredParam<Real>("rho", "density");

  return params;
}

INSChorinPressurePoisson::INSChorinPressurePoisson(const InputParameters & parameters)
  : Kernel(parameters),

    // Gradients
    _grad_u_star(coupledGradient("u_star")),
    _grad_v_star(_mesh.dimension() >= 2 ? coupledGradient("v_star") : _grad_zero),
    _grad_w_star(_mesh.dimension() == 3 ? coupledGradient("w_star") : _grad_zero),

    // Variable numberings
    _u_vel_star_var_number(coupled("u_star")),
    _v_vel_star_var_number(_mesh.dimension() >= 2 ? coupled("v_star") : libMesh::invalid_uint),
    _w_vel_star_var_number(_mesh.dimension() == 3 ? coupled("w_star") : libMesh::invalid_uint),

    // Required parameters
    _rho(getParam<Real>("rho"))
{
}

Real
INSChorinPressurePoisson::computeQpResidual()
{
  // Laplacian part
  Real laplacian_part = _grad_u[_qp] * _grad_test[_i][_qp];

  // Divergence part, don't forget to *divide* by _dt
  Real div_part = (_rho / _dt) *
                  (_grad_u_star[_qp](0) + _grad_v_star[_qp](1) + _grad_w_star[_qp](2)) *
                  _test[_i][_qp];

  // Return the result
  return laplacian_part + div_part;
}

Real
INSChorinPressurePoisson::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
INSChorinPressurePoisson::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_star_var_number)
    return (_rho / _dt) * _grad_phi[_j][_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_star_var_number)
    return (_rho / _dt) * _grad_phi[_j][_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_star_var_number)
    return (_rho / _dt) * _grad_phi[_j][_qp](2) * _test[_i][_qp];

  else
    return 0;
}

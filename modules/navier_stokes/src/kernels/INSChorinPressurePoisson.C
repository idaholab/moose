//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSChorinPressurePoisson.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSChorinPressurePoisson);

InputParameters
INSChorinPressurePoisson::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the pressure Poisson solve which is part of the "
                             "'split' scheme used for solving the incompressible Navier-Stokes "
                             "equations.");
  // Coupled variables
  params.addRequiredCoupledVar("u_star", "star x-velocity");
  params.addCoupledVar("v_star", "star y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w_star", "star z-velocity"); // only required in 3D

  // Optional parameters
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density_name");

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

    // Material properties
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSChorinPressurePoisson::computeQpResidual()
{
  // Laplacian part
  Real laplacian_part = _grad_u[_qp] * _grad_test[_i][_qp];

  // Divergence part, don't forget to *divide* by _dt
  Real div_part = (_rho[_qp] / _dt) *
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
    return (_rho[_qp] / _dt) * _grad_phi[_j][_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_star_var_number)
    return (_rho[_qp] / _dt) * _grad_phi[_j][_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_star_var_number)
    return (_rho[_qp] / _dt) * _grad_phi[_j][_qp](2) * _test[_i][_qp];

  else
    return 0;
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSPressurePoisson.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSPressurePoisson>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("This class computes the pressure Poisson solve which is part of the "
                             "'split' scheme used for solving the incompressible Navier-Stokes "
                             "equations.");
  // Coupled variables
  params.addRequiredCoupledVar("a1", "x-acceleration");
  params.addCoupledVar("a2", "y-acceleration"); // only required in 2D and 3D
  params.addCoupledVar("a3", "z-acceleration"); // only required in 3D

  // Required parameters
  params.addRequiredParam<Real>("rho", "density");

  return params;
}

INSPressurePoisson::INSPressurePoisson(const InputParameters & parameters)
  : Kernel(parameters),

    // Gradients
    _grad_a1(coupledGradient("a1")),
    _grad_a2(_mesh.dimension() >= 2 ? coupledGradient("a2") : _grad_zero),
    _grad_a3(_mesh.dimension() == 3 ? coupledGradient("a3") : _grad_zero),

    // Variable numberings
    _a1_var_number(coupled("a1")),
    _a2_var_number(_mesh.dimension() >= 2 ? coupled("a2") : libMesh::invalid_uint),
    _a3_var_number(_mesh.dimension() == 3 ? coupled("a3") : libMesh::invalid_uint),

    // Required parameters
    _rho(getParam<Real>("rho"))
{
}

Real
INSPressurePoisson::computeQpResidual()
{
  // Laplacian part
  Real laplacian_part = _grad_u[_qp] * _grad_test[_i][_qp];

  // Divergence part
  Real div_part = _rho * (_grad_a1[_qp](0) + _grad_a2[_qp](1) + _grad_a3[_qp](2)) * _test[_i][_qp];

  // Return the result
  return laplacian_part + div_part;
}

Real
INSPressurePoisson::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
INSPressurePoisson::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _a1_var_number)
    return _rho * _grad_phi[_j][_qp](0) * _test[_i][_qp];

  else if (jvar == _a2_var_number)
    return _rho * _grad_phi[_j][_qp](1) * _test[_i][_qp];

  else if (jvar == _a3_var_number)
    return _rho * _grad_phi[_j][_qp](2) * _test[_i][_qp];

  else
    return 0;
}

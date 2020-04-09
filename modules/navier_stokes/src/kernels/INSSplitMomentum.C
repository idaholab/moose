//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSSplitMomentum.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSSplitMomentum);

InputParameters
INSSplitMomentum::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the 'split' momentum equation residual.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  params.addRequiredCoupledVar("a1", "x-acceleration");
  params.addCoupledVar("a2", "y-acceleration"); // only required in 2D and 3D
  params.addCoupledVar("a3", "z-acceleration"); // only required in 3D

  // Required parameters
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  return params;
}

INSSplitMomentum::INSSplitMomentum(const InputParameters & parameters)
  : Kernel(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),

    _a1(coupledValue("a1")),
    _a2(_mesh.dimension() >= 2 ? coupledValue("a2") : _zero),
    _a3(_mesh.dimension() == 3 ? coupledValue("a3") : _zero),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),

    _a1_var_number(coupled("a1")),
    _a2_var_number(_mesh.dimension() >= 2 ? coupled("a2") : libMesh::invalid_uint),
    _a3_var_number(_mesh.dimension() == 3 ? coupled("a3") : libMesh::invalid_uint),

    // Required parameters
    _gravity(getParam<RealVectorValue>("gravity")),
    _component(getParam<unsigned>("component")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSSplitMomentum::computeQpResidual()
{
  // Vector object for a
  RealVectorValue a(_a1[_qp], _a2[_qp], _a3[_qp]);

  // Vector object for test function
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  // Tensor object for "grad U" = d(u_i)/d(x_j)
  RealTensorValue grad_U(_grad_u_vel[_qp], _grad_v_vel[_qp], _grad_w_vel[_qp]);

  // Vector object for U
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Viscous stress tensor object
  RealTensorValue tau(
      // Row 1
      2. * _grad_u_vel[_qp](0),
      _grad_u_vel[_qp](1) + _grad_v_vel[_qp](0),
      _grad_u_vel[_qp](2) + _grad_w_vel[_qp](0),
      // Row 2
      _grad_v_vel[_qp](0) + _grad_u_vel[_qp](1),
      2. * _grad_v_vel[_qp](1),
      _grad_v_vel[_qp](2) + _grad_w_vel[_qp](1),
      // Row 3
      _grad_w_vel[_qp](0) + _grad_u_vel[_qp](2),
      _grad_w_vel[_qp](1) + _grad_v_vel[_qp](2),
      2. * _grad_w_vel[_qp](2));

  // Tensor object for test function gradient
  RealTensorValue grad_test;
  for (unsigned k = 0; k < 3; ++k)
    grad_test(_component, k) = _grad_test[_i][_qp](k);

  //
  // Compute the different pieces...
  //

  // "Symmetric" part, a.test
  Real symmetric_part = a(_component) * _test[_i][_qp];

  // The convection part, (u.grad) * u_component * test_scalar.  Which can also be
  // written as (grad_U * U) * test_vec
  Real convective_part = (grad_U * U) * test;

  // The viscous part, tau : grad(v)
  Real viscous_part = (_mu[_qp] / _rho[_qp]) * tau.contract(grad_test);

  return symmetric_part + convective_part + viscous_part;
}

Real
INSSplitMomentum::computeQpJacobian()
{
  // The on-diagonal Jacobian contribution (derivative of a.test wrt a)
  // is just the mass matrix entry.
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
INSSplitMomentum::computeQpOffDiagJacobian(unsigned jvar)
{
  if ((jvar == _u_vel_var_number) || (jvar == _v_vel_var_number) || (jvar == _w_vel_var_number))
  {
    // Derivative of viscous stress tensor
    RealTensorValue dtau;

    // Initialize to invalid value, then determine correct value.
    unsigned vel_index = 99;

    // Set index and build dtau for that index
    if (jvar == _u_vel_var_number)
    {
      vel_index = 0;
      dtau(0, 0) = 2. * _grad_phi[_j][_qp](0);
      dtau(0, 1) = _grad_phi[_j][_qp](1);
      dtau(0, 2) = _grad_phi[_j][_qp](2);
      dtau(1, 0) = _grad_phi[_j][_qp](1);
      dtau(2, 0) = _grad_phi[_j][_qp](2);
    }
    else if (jvar == _v_vel_var_number)
    {
      vel_index = 1;
      /*                                 */ dtau(0, 1) = _grad_phi[_j][_qp](0);
      dtau(1, 0) = _grad_phi[_j][_qp](0);
      dtau(1, 1) = 2. * _grad_phi[_j][_qp](1);
      dtau(1, 2) = _grad_phi[_j][_qp](2);
      /*                                 */ dtau(2, 1) = _grad_phi[_j][_qp](2);
    }
    else if (jvar == _w_vel_var_number)
    {
      vel_index = 2;
      /*                                                                     */ dtau(0, 2) =
          _grad_phi[_j][_qp](0);
      /*                                                                     */ dtau(1, 2) =
          _grad_phi[_j][_qp](1);
      dtau(2, 0) = _grad_phi[_j][_qp](0);
      dtau(2, 1) = _grad_phi[_j][_qp](1);
      dtau(2, 2) = 2. * _grad_phi[_j][_qp](2);
    }

    // Vector object for test function
    RealVectorValue test;
    test(_component) = _test[_i][_qp];

    // Vector object for U
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

    // Tensor object for test function gradient
    RealTensorValue grad_test;
    for (unsigned k = 0; k < 3; ++k)
      grad_test(_component, k) = _grad_test[_i][_qp](k);

    // Compute the convective part
    RealVectorValue convective_jac = _phi[_j][_qp] * RealVectorValue(_grad_u_vel[_qp](vel_index),
                                                                     _grad_v_vel[_qp](vel_index),
                                                                     _grad_w_vel[_qp](vel_index));

    // Extra contribution in vel_index component
    convective_jac(vel_index) += U * _grad_phi[_j][_qp];
    Real convective_part = convective_jac * test;

    // Compute the viscous part
    Real viscous_part = (_mu[_qp] / _rho[_qp]) * dtau.contract(grad_test);

    // Return the result
    return convective_part + viscous_part;
  }

  else
    return 0;
}

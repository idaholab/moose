/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSMomentumSUPG.h"

template <>
InputParameters
validParams<INSMomentumSUPG>()
{
  InputParameters params = validParams<Kernel>();

  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D

  // Required parameters
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  return params;
}

INSMomentumSUPG::INSMomentumSUPG(const InputParameters & parameters)
  : Kernel(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(coupledValue("w")),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(coupledGradient("v")),
    _grad_w_vel(coupledGradient("w")),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w")),

    // Required parameters
    _component(getParam<unsigned>("component")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSMomentumSUPG::computeQpResidual()
{
  // The convection part, rho * (u.grad) * u_component * v.
  // Note: _grad_u is the gradient of the _component entry of the velocity vector.

  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real Pe = _rho[_qp] * _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
  Real delta = 1. / std::tanh(Pe) - 1. / Pe;
  Real alpha0 = delta * U.norm() * _current_elem->hmax() / 2.;
  Real PG_test = alpha0 / (U.norm() * U.norm()) * U * _grad_test[_i][_qp];
  Real convective_part = _rho[_qp] *
                         (_u_vel[_qp] * _grad_u[_qp](0) + _v_vel[_qp] * _grad_u[_qp](1) +
                          _w_vel[_qp] * _grad_u[_qp](2)) *
                         PG_test;

  return convective_part;
}

Real
INSMomentumSUPG::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real Pe = _rho[_qp] * _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
  Real delta = 1. / std::tanh(Pe) - 1. / Pe;
  Real alpha0 = delta * U.norm() * _current_elem->hmax() / 2.;
  Real PG_test = alpha0 / (U.norm() * U.norm()) * U * _grad_test[_i][_qp];

  // Convective part
  Real convective_part =
      _rho[_qp] * ((U * _grad_phi[_j][_qp]) + _phi[_j][_qp] * _grad_u[_qp](_component)) * PG_test;

  return convective_part;
}

Real
INSMomentumSUPG::computeQpOffDiagJacobian(unsigned jvar)
{
  // In Stokes/Laplacian version, off-diag Jacobian entries wrt u,v,w are zero
  if (jvar == _u_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real Pe = _rho[_qp] * _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
    Real delta = 1. / std::tanh(Pe) - 1. / Pe;
    Real alpha0 = delta * U.norm() * _current_elem->hmax() / 2.;
    Real PG_test = alpha0 / (U.norm() * U.norm()) * U * _grad_test[_i][_qp];

    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](0) * PG_test;

    return convective_part;
  }

  else if (jvar == _v_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real Pe = _rho[_qp] * _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
    Real delta = 1. / std::tanh(Pe) - 1. / Pe;
    Real alpha0 = delta * U.norm() * _current_elem->hmax() / 2.;
    Real PG_test = alpha0 / (U.norm() * U.norm()) * U * _grad_test[_i][_qp];

    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](1) * PG_test;

    return convective_part;
  }

  else if (jvar == _w_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real Pe = _rho[_qp] * _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
    Real delta = 1. / std::tanh(Pe) - 1. / Pe;
    Real alpha0 = delta * U.norm() * _current_elem->hmax() / 2.;
    Real PG_test = alpha0 / (U.norm() * U.norm()) * U * _grad_test[_i][_qp];

    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](2) * PG_test;

    return convective_part;
  }

  else
    return 0;
}

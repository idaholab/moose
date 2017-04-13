/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSMomentumBase.h"

template <>
InputParameters
validParams<INSMomentumBase>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("This class computes the spatial part of the momentum equation "
                             "residual and Jacobian for the incompressible Navier-Stokes momentum "
                             "equation, calling a virtual function to get the viscous "
                             "contribution.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  // Required parameters
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");
  params.addParam<bool>("integrate_p_by_parts",
                        true,
                        "Allows simulations to be run with pressure BC if set to false");

  return params;
}

INSMomentumBase::INSMomentumBase(const InputParameters & parameters)
  : Kernel(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(coupledValue("w")),
    _p(coupledValue("p")),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(coupledGradient("v")),
    _grad_w_vel(coupledGradient("w")),
    _grad_p(coupledGradient("p")),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w")),
    _p_var_number(coupled("p")),

    // Required parameters
    _mu(getParam<Real>("mu")),
    _rho(getParam<Real>("rho")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _component(getParam<unsigned>("component")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts"))

// Material properties
// _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity"))
{
}

Real
INSMomentumBase::computeQpResidual()
{
  // The convection part, rho * (u.grad) * u_component * v.
  // Note: _grad_u is the gradient of the _component entry of the velocity vector.
  Real convective_part = _rho * (_u_vel[_qp] * _grad_u[_qp](0) + _v_vel[_qp] * _grad_u[_qp](1) +
                                 _w_vel[_qp] * _grad_u[_qp](2)) *
                         _test[_i][_qp];

  // The pressure part, -p (div v) or (dp/dx_{component}) * test if not integrated by parts.
  Real pressure_part = 0.;
  if (_integrate_p_by_parts)
    pressure_part = -_p[_qp] * _grad_test[_i][_qp](_component);
  else
    pressure_part = _grad_p[_qp](_component) * _test[_i][_qp];

  // Call derived class to compute the viscous contribution.
  Real viscous_part = computeQpResidualViscousPart();

  // Body force term.  For truly incompressible flow, this term is constant, and
  // since it is proportional to g, can be written as the gradient of some scalar
  // and absorbed into the pressure definition.
  // Real body_force_part = - _rho * _gravity(_component);

  return convective_part + pressure_part + viscous_part /*+ body_force_part*/;
}

Real
INSMomentumBase::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Convective part
  Real convective_part =
      _rho * ((U * _grad_phi[_j][_qp]) + _phi[_j][_qp] * _grad_u[_qp](_component)) * _test[_i][_qp];

  // Call derived class to compute the viscous contribution.
  Real viscous_part = computeQpJacobianViscousPart();

  return convective_part + viscous_part;
}

Real
INSMomentumBase::computeQpOffDiagJacobian(unsigned jvar)
{
  // In Stokes/Laplacian version, off-diag Jacobian entries wrt u,v,w are zero
  if (jvar == _u_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](0) * _test[_i][_qp];

    // Call derived class to compute the viscous contribution.
    Real viscous_part = computeQpOffDiagJacobianViscousPart(jvar);

    return convective_part + viscous_part;
  }

  else if (jvar == _v_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](1) * _test[_i][_qp];

    // Call derived class to compute the viscous contribution.
    Real viscous_part = computeQpOffDiagJacobianViscousPart(jvar);

    return convective_part + viscous_part;
  }

  else if (jvar == _w_vel_var_number)
  {
    Real convective_part = _phi[_j][_qp] * _grad_u[_qp](2) * _test[_i][_qp];

    // Call derived class to compute the viscous contribution.
    Real viscous_part = computeQpOffDiagJacobianViscousPart(jvar);

    return convective_part + viscous_part;
  }

  else if (jvar == _p_var_number)
  {
    if (_integrate_p_by_parts)
      return -_phi[_j][_qp] * _grad_test[_i][_qp](_component);
    else
      return _grad_phi[_j][_qp](_component) * _test[_i][_qp];
  }

  else
    return 0;
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSBase.h"
#include "Assembly.h"

template <>
InputParameters
validParams<INSBase>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("This class computes various strong and weak components of the "
                             "incompressible navier stokes equations which can then be assembled "
                             "together in child classes.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  // Required parameters
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<bool>(
      "stokes_only", false, "Whether to ignore the convective acceleration term.");

  return params;
}

INSBase::INSBase(const InputParameters & parameters)
  : Kernel(parameters),
    _second_phi(_assembly.secondPhi()),

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

    // second derivative tensors
    _second_u_vel(coupledSecond("u")),
    _second_v_vel(coupledSecond("v")),
    _second_w_vel(coupledSecond("w")),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w")),
    _p_var_number(coupled("p")),

    // Required parameters
    _gravity(getParam<RealVectorValue>("gravity")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name")),

    _stokes_only(getParam<bool>("stokes_only"))
{
}

RealVectorValue
INSBase::computeStrongConvectiveTerm()
{
  RealVectorValue convective_term(0, 0, 0);
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  convective_term(0) = _rho[_qp] * U * _grad_u_vel[_qp];
  convective_term(1) = _rho[_qp] * U * _grad_v_vel[_qp];
  convective_term(2) = _rho[_qp] * U * _grad_w_vel[_qp];

  return convective_term;
}

RealVectorValue
INSBase::dConvecDUComp(unsigned comp)
{
  RealVectorValue convective_term(0, 0, 0);
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  RealVectorValue d_U_d_comp(0, 0, 0);
  d_U_d_comp(comp) = _phi[_j][_qp];
  convective_term(0) = _rho[_qp] * d_U_d_comp * _grad_u_vel[_qp];
  convective_term(1) = _rho[_qp] * d_U_d_comp * _grad_v_vel[_qp];
  convective_term(2) = _rho[_qp] * d_U_d_comp * _grad_w_vel[_qp];

  convective_term(comp) += _rho[_qp] * U * _grad_phi[_j][_qp];

  return convective_term;
}

RealVectorValue
INSBase::computeStrongViscousTerm()
{
  RealVectorValue viscous_term(0, 0, 0);
  Real lap_u = _second_u_vel[_qp](0, 0) + _second_u_vel[_qp](1, 1) + _second_u_vel[_qp](2, 2);
  Real lap_v = _second_v_vel[_qp](0, 0) + _second_v_vel[_qp](1, 1) + _second_v_vel[_qp](2, 2);
  Real lap_w = _second_w_vel[_qp](0, 0) + _second_w_vel[_qp](1, 1) + _second_w_vel[_qp](2, 2);
  viscous_term(0) = -_mu[_qp] * lap_u;
  viscous_term(1) = -_mu[_qp] * lap_v;
  viscous_term(2) = -_mu[_qp] * lap_w;

  return viscous_term;
}

RealVectorValue
INSBase::dViscDUComp(unsigned comp)
{
  RealVectorValue viscous_term(0, 0, 0);
  viscous_term(comp) = -_mu[_qp] * (_second_phi[_j][_qp](0, 0) + _second_phi[_j][_qp](1, 1) +
                                    _second_phi[_j][_qp](2, 2));

  return viscous_term;
}

RealVectorValue
INSBase::weakViscousTerm(unsigned comp)
{
  switch (comp)
  {
    case 0:
      return _mu[_qp] * _grad_u_vel[_qp];

    case 1:
      return _mu[_qp] * _grad_v_vel[_qp];

    case 2:
      return _mu[_qp] * _grad_w_vel[_qp];

    default:
      return _zero[_qp];
  }
}

RealVectorValue
INSBase::dWeakViscDUComp()
{
  return _mu[_qp] * _grad_phi[_j][_qp];
}

RealVectorValue
INSBase::computeStrongPressureTerm()
{
  return _grad_p[_qp];
}

Real
INSBase::computeWeakPressureTerm()
{
  return -_p[_qp];
}

RealVectorValue
INSBase::dPressureDPressure()
{
  return _grad_phi[_j][_qp];
}

RealVectorValue
INSBase::computeStrongGravityTerm()
{
  return -_rho[_qp] * _gravity;
}

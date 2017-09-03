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

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Direction of the gravity vector");

  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  params.addParam<bool>("stabilize", false, "Whether to use GLS (encompasses SUPG) stabilization.");
  params.addParam<Real>("alpha", 1., "Multiplicative factor on the stabilization parameter tau.");
  params.addParam<bool>(
      "laplace", true, "Whether the viscous term of the momentum equations is in laplace form.");
  params.addParam<bool>("convective_term", true, "Whether to include the convective term.");

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

    _gravity(getParam<RealVectorValue>("gravity")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name")),

    _stabilize(getParam<bool>("stabilize")),
    _alpha(getParam<Real>("alpha")),
    _laplace(getParam<bool>("laplace")),
    _convective_term(getParam<bool>("convective_term"))
{
}

RealVectorValue
INSBase::convectiveTerm()
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
INSBase::strongViscousTermLaplace()
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
INSBase::strongViscousTermTraction()
{
  RealVectorValue viscous_term = strongViscousTermLaplace();

  viscous_term(0) +=
      -_mu[_qp] * (_second_u_vel[_qp](0, 0) + _second_v_vel[_qp](0, 1) + _second_w_vel[_qp](0, 2));
  viscous_term(1) +=
      -_mu[_qp] * (_second_u_vel[_qp](1, 0) + _second_v_vel[_qp](1, 1) + _second_w_vel[_qp](1, 2));
  viscous_term(2) +=
      -_mu[_qp] * (_second_u_vel[_qp](2, 0) + _second_v_vel[_qp](2, 1) + _second_w_vel[_qp](2, 2));

  return viscous_term;
}

RealVectorValue
INSBase::dStrongViscDUCompLaplace(unsigned comp)
{
  RealVectorValue viscous_term(0, 0, 0);
  viscous_term(comp) = -_mu[_qp] * (_second_phi[_j][_qp](0, 0) + _second_phi[_j][_qp](1, 1) +
                                    _second_phi[_j][_qp](2, 2));

  return viscous_term;
}

// Fix me
RealVectorValue
INSBase::dStrongViscDUCompTraction(unsigned comp)
{
  RealVectorValue viscous_term(0, 0, 0);
  viscous_term(comp) = -_mu[_qp] * (_second_phi[_j][_qp](0, 0) + _second_phi[_j][_qp](1, 1) +
                                    _second_phi[_j][_qp](2, 2));

  return viscous_term;
}

RealVectorValue
INSBase::weakViscousTermLaplace(unsigned comp)
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
INSBase::weakViscousTermTraction(unsigned comp)
{
  switch (comp)
  {
    case 0:
    {
      RealVectorValue transpose(_grad_u_vel[_qp](0), _grad_v_vel[_qp](0), _grad_w_vel[_qp](0));
      return _mu[_qp] * _grad_u_vel[_qp] + _mu[_qp] * transpose;
    }

    case 1:
    {
      RealVectorValue transpose(_grad_u_vel[_qp](1), _grad_v_vel[_qp](1), _grad_w_vel[_qp](1));
      return _mu[_qp] * _grad_v_vel[_qp] + _mu[_qp] * transpose;
    }

    case 2:
    {
      RealVectorValue transpose(_grad_u_vel[_qp](2), _grad_v_vel[_qp](2), _grad_w_vel[_qp](2));
      return _mu[_qp] * _grad_w_vel[_qp] + _mu[_qp] * transpose;
    }

    default:
      return _zero[_qp];
  }
}

RealVectorValue
INSBase::dWeakViscDUCompLaplace()
{
  return _mu[_qp] * _grad_phi[_j][_qp];
}

// Fix me
RealVectorValue
INSBase::dWeakViscDUCompTraction()
{
  return _mu[_qp] * _grad_phi[_j][_qp];
}

RealVectorValue
INSBase::strongPressureTerm()
{
  return _grad_p[_qp];
}

Real
INSBase::weakPressureTerm()
{
  return -_p[_qp];
}

RealVectorValue
INSBase::dStrongPressureDPressure()
{
  return _grad_phi[_j][_qp];
}

Real
INSBase::dWeakPressureDPressure()
{
  return -_phi[_j][_qp];
}

RealVectorValue
INSBase::bodyForcesTerm()
{
  return gravity();
}

RealVectorValue
INSBase::gravity()
{
  return -_rho[_qp] * _gravity;
}

Real
INSBase::tau()
{
  Real nu = _mu[_qp] / _rho[_qp];
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real h = _current_elem->hmax();
  return _alpha / std::sqrt((2. * U.norm() / h) * (2. * U.norm() / h) +
                            9. * (4. * nu / (h * h)) * (4. * nu / (h * h)));
}

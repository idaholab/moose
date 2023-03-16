//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSBase.h"
#include "Assembly.h"
#include "NS.h"

InputParameters
INSBase::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes various strong and weak components of the "
                             "incompressible navier stokes equations which can then be assembled "
                             "together in child classes.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Direction of the gravity vector");

  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  params.addParam<Real>("alpha", 1., "Multiplicative factor on the stabilization parameter tau.");
  params.addParam<bool>(
      "laplace", true, "Whether the viscous term of the momentum equations is in laplace form.");
  params.addParam<bool>("convective_term", true, "Whether to include the convective term.");
  params.addParam<bool>("transient_term",
                        false,
                        "Whether there should be a transient term in the momentum residuals.");

  return params;
}

INSBase::INSBase(const InputParameters & parameters)
  : Kernel(parameters),
    _second_phi(_assembly.secondPhi()),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(coupledValue("w")),
    _p(coupledValue(NS::pressure)),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(coupledGradient("v")),
    _grad_w_vel(coupledGradient("w")),
    _grad_p(coupledGradient(NS::pressure)),

    // second derivative tensors
    _second_u_vel(coupledSecond("u")),
    _second_v_vel(coupledSecond("v")),
    _second_w_vel(coupledSecond("w")),

    // time derivatives
    _u_vel_dot(_is_transient ? coupledDot("u") : _zero),
    _v_vel_dot(_is_transient ? coupledDot("v") : _zero),
    _w_vel_dot(_is_transient ? coupledDot("w") : _zero),

    // derivatives of time derivatives
    _d_u_vel_dot_du(_is_transient ? coupledDotDu("u") : _zero),
    _d_v_vel_dot_dv(_is_transient ? coupledDotDu("v") : _zero),
    _d_w_vel_dot_dw(_is_transient ? coupledDotDu("w") : _zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w")),
    _p_var_number(coupled(NS::pressure)),

    _gravity(getParam<RealVectorValue>("gravity")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name")),

    _alpha(getParam<Real>("alpha")),
    _laplace(getParam<bool>("laplace")),
    _convective_term(getParam<bool>("convective_term")),
    _transient_term(getParam<bool>("transient_term"))
{
}

RealVectorValue
INSBase::convectiveTerm()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return _rho[_qp] *
         RealVectorValue(U * _grad_u_vel[_qp], U * _grad_v_vel[_qp], U * _grad_w_vel[_qp]);
}

RealVectorValue
INSBase::dConvecDUComp(unsigned comp)
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  RealVectorValue d_U_d_comp(0, 0, 0);
  d_U_d_comp(comp) = _phi[_j][_qp];

  RealVectorValue convective_term = _rho[_qp] * RealVectorValue(d_U_d_comp * _grad_u_vel[_qp],
                                                                d_U_d_comp * _grad_v_vel[_qp],
                                                                d_U_d_comp * _grad_w_vel[_qp]);
  convective_term(comp) += _rho[_qp] * U * _grad_phi[_j][_qp];

  return convective_term;
}

RealVectorValue
INSBase::strongViscousTermLaplace()
{
  return -_mu[_qp] *
         RealVectorValue(_second_u_vel[_qp].tr(), _second_v_vel[_qp].tr(), _second_w_vel[_qp].tr());
}

RealVectorValue
INSBase::strongViscousTermTraction()
{
  return strongViscousTermLaplace() -
         _mu[_qp] *
             (_second_u_vel[_qp].row(0) + _second_v_vel[_qp].row(1) + _second_w_vel[_qp].row(2));
}

RealVectorValue
INSBase::dStrongViscDUCompLaplace(unsigned comp)
{
  RealVectorValue viscous_term(0, 0, 0);
  viscous_term(comp) = -_mu[_qp] * _second_phi[_j][_qp].tr();

  return viscous_term;
}

RealVectorValue
INSBase::dStrongViscDUCompTraction(unsigned comp)
{
  RealVectorValue viscous_term(0, 0, 0);
  viscous_term(comp) = -_mu[_qp] * (_second_phi[_j][_qp](0, 0) + _second_phi[_j][_qp](1, 1) +
                                    _second_phi[_j][_qp](2, 2));
  for (unsigned i = 0; i < 3; i++)
    viscous_term(i) += -_mu[_qp] * _second_phi[_j][_qp](i, comp);

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
INSBase::gravityTerm()
{
  return -_rho[_qp] * _gravity;
}

RealVectorValue
INSBase::timeDerivativeTerm()
{
  return _rho[_qp] * RealVectorValue(_u_vel_dot[_qp], _v_vel_dot[_qp], _w_vel_dot[_qp]);
}

RealVectorValue
INSBase::dTimeDerivativeDUComp(unsigned comp)
{
  Real base = _rho[_qp] * _phi[_j][_qp];
  switch (comp)
  {
    case 0:
      return RealVectorValue(base * _d_u_vel_dot_du[_qp], 0, 0);

    case 1:
      return RealVectorValue(0, base * _d_v_vel_dot_dv[_qp], 0);

    case 2:
      return RealVectorValue(0, 0, base * _d_w_vel_dot_dw[_qp]);

    default:
      mooseError("comp must be 0, 1, or 2");
  }
}

Real
INSBase::tau()
{
  Real nu = _mu[_qp] / _rho[_qp];
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real h = _current_elem->hmax();
  Real transient_part = _transient_term ? 4. / (_dt * _dt) : 0.;
  return _alpha / std::sqrt(transient_part + (2. * U.norm() / h) * (2. * U.norm() / h) +
                            9. * (4. * nu / (h * h)) * (4. * nu / (h * h)));
}

Real
INSBase::tauNodal()
{
  Real nu = _mu[_qp] / _rho[_qp];
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real h = _current_elem->hmax();
  Real xi;
  if (nu < std::numeric_limits<Real>::epsilon())
    xi = 1;
  else
  {
    Real alpha = U.norm() * h / (2 * nu);
    xi = 1. / std::tanh(alpha) - 1. / alpha;
  }
  return h / (2 * U.norm()) * xi;
}

Real
INSBase::dTauDUComp(unsigned comp)
{
  Real nu = _mu[_qp] / _rho[_qp];
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real h = _current_elem->hmax();
  Real transient_part = _transient_term ? 4. / (_dt * _dt) : 0.;
  return -_alpha / 2. *
         std::pow(transient_part + (2. * U.norm() / h) * (2. * U.norm() / h) +
                      9. * (4. * nu / (h * h)) * (4. * nu / (h * h)),
                  -1.5) *
         2. * (2. * U.norm() / h) * 2. / h * U(comp) * _phi[_j][_qp] /
         (U.norm() + std::numeric_limits<double>::epsilon());
}

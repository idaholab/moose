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
  params.addRequiredCoupledVar("p", "pressure");

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

INSMomentumSUPG::INSMomentumSUPG(const InputParameters & parameters)
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

    // parameters
    _gravity(getParam<RealVectorValue>("gravity")),
    _component(getParam<unsigned>("component")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSMomentumSUPG::computeQpResidual()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real alpha;
  if (_mu[_qp] < std::numeric_limits<double>::epsilon())
    alpha = 1;
  else if (U.norm() < std::numeric_limits<double>::epsilon())
  {
    alpha = 0;
    return 0;
  }
  else
  {
    Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
    alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
  }
  Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];

  Real convective_part = _rho[_qp] * U * _grad_u[_qp] * PG_test;

  // Note that integrating pressure by parts would imply applying a second
  // derivative to the test function, which we are not equipped to do (?)
  Real pressure_part = _grad_p[_qp](_component) * PG_test;

  // From Computer Mechanics in Applied Mechanics and Engineering 32 (1982) pg. 212:
  // For reasonable (linear) element shapes the contribution of PG_test * viscous
  // term is small and can be neglected. This is not generally the case for higher
  // order elements (and note that we commonly use quadratic shape functions for
  // velocity). However, including this term would again imply applying a second
  // derivative either to the test function or to the variable u
  // Real viscous_part = computeQpResidualViscousPart();

  Real body_force_part = -_rho[_qp] * _gravity(_component) * PG_test;

  return convective_part + pressure_part + /*viscous_part +*/ body_force_part;
}

Real
INSMomentumSUPG::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real alpha;
  Real d_alpha_d_vel_component;

  // Pure advection
  if (_mu[_qp] < std::numeric_limits<double>::epsilon())
  {
    alpha = 1;
    d_alpha_d_vel_component = 0;
  }
  // Pure diffusion
  else if (U.norm() < std::numeric_limits<double>::epsilon())
  {
    alpha = 0;
    return 0;
  }
  else
  {
    Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
    Real d_grid_Pe_d_vel_component =
        _current_elem->hmax() * _u[_qp] * _phi[_j][_qp] / (2. * _mu[_qp] * U.norm());
    alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
    d_alpha_d_vel_component =
        ((1. - 1. / (std::tanh(grid_Pe) * std::tanh(grid_Pe))) + 1. / (grid_Pe * grid_Pe)) *
        d_grid_Pe_d_vel_component;
  }
  Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];
  RealVectorValue d_U_d_vel_component(0, 0, 0);
  d_U_d_vel_component(_component) = _phi[_j][_qp];
  Real d_PG_test_d_vel_component =
      _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
      (d_alpha_d_vel_component * U / U.norm() +
       alpha * (d_U_d_vel_component / U.norm() -
                U / (U.norm() * U.norm() * U.norm()) * _u[_qp] * _phi[_j][_qp]));

  Real convective_part =
      _rho[_qp] * ((U * _grad_phi[_j][_qp]) + _phi[_j][_qp] * _grad_u[_qp](_component)) * PG_test;
  convective_part += _rho[_qp] * U * _grad_u[_qp] * d_PG_test_d_vel_component;

  Real pressure_part = _grad_p[_qp](_component) * d_PG_test_d_vel_component;

  Real body_force_part = -_rho[_qp] * _gravity(_component) * d_PG_test_d_vel_component;

  return convective_part + pressure_part + body_force_part;
}

Real
INSMomentumSUPG::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real alpha;
    Real d_alpha_d_vel_component;
    if (_mu[_qp] < std::numeric_limits<double>::epsilon())
    {
      alpha = 1;
      d_alpha_d_vel_component = 0;
    }
    else if (U.norm() < std::numeric_limits<double>::epsilon())
    {
      alpha = 0;
      return 0;
    }
    else
    {
      Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
      Real d_grid_Pe_d_vel_component =
          _current_elem->hmax() * _u_vel[_qp] * _phi[_j][_qp] / (2. * _mu[_qp] * U.norm());
      alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
      d_alpha_d_vel_component =
          ((1. - 1. / (std::tanh(grid_Pe) * std::tanh(grid_Pe))) + 1. / (grid_Pe * grid_Pe)) *
          d_grid_Pe_d_vel_component;
    }
    Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(0) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _u_vel[_qp] * _phi[_j][_qp]));

    Real convective_part = _rho[_qp] * _phi[_j][_qp] * _grad_u[_qp](0) * PG_test;
    convective_part += _rho[_qp] * U * _grad_u[_qp] * d_PG_test_d_vel_component;

    Real pressure_part = _grad_p[_qp](_component) * d_PG_test_d_vel_component;

    Real body_force_part = -_rho[_qp] * _gravity(_component) * d_PG_test_d_vel_component;

    return convective_part + pressure_part + body_force_part;
  }

  else if (jvar == _v_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real alpha;
    Real d_alpha_d_vel_component;
    if (_mu[_qp] < std::numeric_limits<double>::epsilon())
    {
      alpha = 1;
      d_alpha_d_vel_component = 0;
    }
    else if (U.norm() < std::numeric_limits<double>::epsilon())
    {
      alpha = 0;
      return 0;
    }
    else
    {
      Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
      Real d_grid_Pe_d_vel_component =
          _current_elem->hmax() * _v_vel[_qp] * _phi[_j][_qp] / (2. * _mu[_qp] * U.norm());
      alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
      d_alpha_d_vel_component =
          ((1. - 1. / (std::tanh(grid_Pe) * std::tanh(grid_Pe))) + 1. / (grid_Pe * grid_Pe)) *
          d_grid_Pe_d_vel_component;
    }
    Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(1) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _v_vel[_qp] * _phi[_j][_qp]));

    Real convective_part = _rho[_qp] * _phi[_j][_qp] * _grad_u[_qp](1) * PG_test;
    convective_part += _rho[_qp] * U * _grad_u[_qp] * d_PG_test_d_vel_component;

    Real pressure_part = _grad_p[_qp](_component) * d_PG_test_d_vel_component;

    Real body_force_part = -_rho[_qp] * _gravity(_component) * d_PG_test_d_vel_component;

    return convective_part + pressure_part + body_force_part;
  }

  else if (jvar == _w_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real alpha;
    Real d_alpha_d_vel_component;
    if (_mu[_qp] < std::numeric_limits<double>::epsilon())
    {
      alpha = 1;
      d_alpha_d_vel_component = 0;
    }
    else if (U.norm() < std::numeric_limits<double>::epsilon())
    {
      alpha = 0;
      return 0;
    }
    else
    {
      Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
      Real d_grid_Pe_d_vel_component =
          _current_elem->hmax() * _w_vel[_qp] * _phi[_j][_qp] / (2. * _mu[_qp] * U.norm());
      alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
      d_alpha_d_vel_component =
          ((1. - 1. / (std::tanh(grid_Pe) * std::tanh(grid_Pe))) + 1. / (grid_Pe * grid_Pe)) *
          d_grid_Pe_d_vel_component;
    }
    Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(2) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _w_vel[_qp] * _phi[_j][_qp]));

    Real convective_part = _rho[_qp] * _phi[_j][_qp] * _grad_u[_qp](2) * PG_test;
    convective_part += _rho[_qp] * U * _grad_u[_qp] * d_PG_test_d_vel_component;

    Real pressure_part = _grad_p[_qp](_component) * d_PG_test_d_vel_component;

    Real body_force_part = -_rho[_qp] * _gravity(_component) * d_PG_test_d_vel_component;

    return convective_part + pressure_part + body_force_part;
  }

  else if (jvar == _p_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real alpha;
    if (_mu[_qp] < std::numeric_limits<double>::epsilon())
      alpha = 1;
    else if (U.norm() < std::numeric_limits<double>::epsilon())
    {
      alpha = 0;
      return 0;
    }
    else
    {
      Real grid_Pe = _current_elem->hmax() * U.norm() / (2. * _mu[_qp]);
      alpha = 1. / std::tanh(grid_Pe) - 1. / grid_Pe;
    }
    Real PG_test = alpha * _current_elem->hmax() / 2. * U / U.norm() * _grad_test[_i][_qp];

    Real pressure_part = _grad_phi[_j][_qp](_component) * PG_test;
    return pressure_part;
  }

  else
    return 0;
}

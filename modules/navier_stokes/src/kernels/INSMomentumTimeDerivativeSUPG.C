/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumTimeDerivativeSUPG.h"

template <>
InputParameters
validParams<INSMomentumTimeDerivativeSUPG>()
{
  InputParameters params = validParams<INSMomentumTimeDerivative>();
  params.addClassDescription(
      "This class computes the time derivative for the incompressible "
      "Navier-Stokes momentum equation with Petrov-Galerkin test functions.");

  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D

  // Mat props
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addRequiredParam<unsigned>("component",
                                    "The velocity component that this kernel is "
                                    "applied to. Necessary for building correct "
                                    "Jacobians.");
  return params;
}

INSMomentumTimeDerivativeSUPG::INSMomentumTimeDerivativeSUPG(const InputParameters & parameters)
  : INSMomentumTimeDerivative(parameters),
    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(coupledValue("w")),
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w")),

    // Mat props
    _mu(getMaterialProperty<Real>("mu_name")),
    _component(getParam<unsigned>("component"))
{
}

Real
INSMomentumTimeDerivativeSUPG::computeQpResidual()
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

  return _rho[_qp] * _u_dot[_qp] * PG_test;
}

Real
INSMomentumTimeDerivativeSUPG::computeQpJacobian()
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

  Real jac = _rho[_qp] * _du_dot_du[_qp] * _phi[_j][_qp] * PG_test;
  jac += _rho[_qp] * _u_dot[_qp] * d_PG_test_d_vel_component;
  return jac;
}

Real
INSMomentumTimeDerivativeSUPG::computeQpOffDiagJacobian(unsigned jvar)
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
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(0) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _u_vel[_qp] * _phi[_j][_qp]));

    return _rho[_qp] * _u_dot[_qp] * d_PG_test_d_vel_component;
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
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(1) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _v_vel[_qp] * _phi[_j][_qp]));

    return _rho[_qp] * _u_dot[_qp] * d_PG_test_d_vel_component;
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
    RealVectorValue d_U_d_vel_component(0, 0, 0);
    d_U_d_vel_component(2) = _phi[_j][_qp];
    Real d_PG_test_d_vel_component =
        _current_elem->hmax() / 2. * _grad_test[_i][_qp] *
        (d_alpha_d_vel_component * U / U.norm() +
         alpha * (d_U_d_vel_component / U.norm() -
                  U / (U.norm() * U.norm() * U.norm()) * _w_vel[_qp] * _phi[_j][_qp]));

    return _rho[_qp] * _u_dot[_qp] * d_PG_test_d_vel_component;
  }
  else
    return 0;
}

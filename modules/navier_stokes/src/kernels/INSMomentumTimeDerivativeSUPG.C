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
  return params;
}

INSMomentumTimeDerivativeSUPG::INSMomentumTimeDerivativeSUPG(const InputParameters & parameters)
  : INSMomentumTimeDerivative(parameters),
    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(coupledValue("w")),

    // Mat props
    _mu(getMaterialProperty<Real>("mu_name"))
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

  return INSMomentumTimeDerivative::computeQpResidual() + _rho[_qp] * _u_dot[_qp] * PG_test;
}

Real
INSMomentumTimeDerivativeSUPG::computeQpJacobian()
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

  return INSMomentumTimeDerivative::computeQpJacobian() +
         _rho[_qp] * _du_dot_du[_qp] * _phi[_j][_qp] * PG_test;
}

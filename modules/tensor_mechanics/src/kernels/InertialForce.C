/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InertialForce.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<InertialForce>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Calculates the residual for the interial force (M*accel) and the "
                             "contribution of mass dependent Rayleigh damping and HHT time "
                             "integration scheme [eta*M*((1+alpha)vel-alpha*vel_old)]");
  params.set<bool>("use_displaced_mesh") = true;
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredCoupledVar("acceleration", "acceleration variable");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark Time integration");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for the "
                                        "Rayleigh damping.");
  params.addParam<Real>("alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  return params;
}

InertialForce::InertialForce(const InputParameters & parameters)
  : Kernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _u_old(valueOld()),
    _vel_old(coupledValueOld("velocity")),
    _accel_old(coupledValueOld("acceleration")),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha"))
{
}

Real
InertialForce::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
  {
    Real accel = 1. / _beta * (((_u[_qp] - _u_old[_qp]) / (_dt * _dt)) - _vel_old[_qp] / _dt -
                               _accel_old[_qp] * (0.5 - _beta));
    Real vel = _vel_old[_qp] + (_dt * (1 - _gamma)) * _accel_old[_qp] + _gamma * _dt * accel;
    return _test[_i][_qp] * _density[_qp] *
           (accel + vel * _eta[_qp] * (1 + _alpha) - _alpha * _eta[_qp] * _vel_old[_qp]);
  }
}

Real
InertialForce::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
    return _test[_i][_qp] * _density[_qp] / (_beta * _dt * _dt) * _phi[_j][_qp] +
           _eta[_qp] * (1 + _alpha) * _test[_i][_qp] * _density[_qp] * _gamma / _beta / _dt *
               _phi[_j][_qp];
}

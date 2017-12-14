/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalInertialForce.h"
#include "MooseVariable.h"
#include "AuxiliarySystem.h"

template <>
InputParameters
validParams<NodalInertialForce>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredCoupledVar("acceleration", "acceleration variable");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark Time integration");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark Time integration");
  params.addParam<Real>("eta",
                        0.0,
                        "Name of material property or a constant real "
                        "number defining the eta parameter for the "
                        "Rayleigh damping.");
  params.addParam<Real>("alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  params.addRequiredParam<Real>("mass", "Mass associated with the node");
  return params;
}

NodalInertialForce::NodalInertialForce(const InputParameters & parameters)
  : NodalKernel(parameters),
    _mass(getParam<Real>("mass")),
    _u_old(_var.nodalSlnOld()),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _eta(getParam<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _aux_sys(_fe_problem.getAuxiliarySystem())
{
  MooseVariable * vel_variable = getVar("velocity", 0);
  _vel_num = vel_variable->number();
  MooseVariable * accel_variable = getVar("acceleration", 0);
  _accel_num = accel_variable->number();
}

Real
NodalInertialForce::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
  {
    const NumericVector<Number> & aux_sol_old = _aux_sys.solutionOld();

    Real vel_old = aux_sol_old(_current_node->dof_number(_aux_sys.number(), _vel_num, 0));
    Real accel_old = aux_sol_old(_current_node->dof_number(_aux_sys.number(), _accel_num, 0));

    Real accel = 1. / _beta * (((_u[_qp] - _u_old[_qp]) / (_dt * _dt)) - vel_old / _dt -
                               accel_old * (0.5 - _beta));
    Real vel = vel_old + (_dt * (1 - _gamma)) * accel_old + _gamma * _dt * accel;
    return _mass * (accel + vel * _eta * (1 + _alpha) - _alpha * _eta * vel_old);
  }
}

Real
NodalInertialForce::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
    return _mass / (_beta * _dt * _dt) + _eta * (1 + _alpha) * _mass * _gamma / _beta / _dt;
}

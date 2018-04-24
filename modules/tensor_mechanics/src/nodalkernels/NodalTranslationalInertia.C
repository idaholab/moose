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

#include "NodalTranslationalInertia.h"
#include "MooseVariable.h"
#include "AuxiliarySystem.h"

registerMooseObject("TensorMechanicsApp", NodalTranslationalInertia);

template <>
InputParameters
validParams<NodalTranslationalInertia>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addClassDescription("Computes the interial forces and mass proportional damping terms "
                             "corresponding to nodal mass.");
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredCoupledVar("acceleration", "acceleration variable");
  params.addRequiredRangeCheckedParam<Real>(
      "beta", "beta>0.0", "beta parameter for Newmark Time integration");
  params.addRequiredRangeCheckedParam<Real>(
      "gamma", "gamma>0.0", "gamma parameter for Newmark Time integration");
  params.addRangeCheckedParam<Real>("eta",
                                    0.0,
                                    "eta>=0.0",
                                    "Constant real number defining the eta parameter for "
                                    "Rayleigh damping.");
  params.addRangeCheckedParam<Real>("alpha",
                                    0.0,
                                    "alpha >= -0.3333 & alpha <= 0.0",
                                    "Alpha parameter for mass dependent numerical damping induced "
                                    "by HHT time integration scheme");
  params.addRequiredParam<Real>("mass", "Mass associated with the node");
  return params;
}

NodalTranslationalInertia::NodalTranslationalInertia(const InputParameters & parameters)
  : NodalKernel(parameters),
    _mass(getParam<Real>("mass")),
    _u_old(_var.dofValuesOld()),
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
NodalTranslationalInertia::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
  {
    const NumericVector<Number> & aux_sol_old = _aux_sys.solutionOld();

    mooseAssert(_beta > 0.0, "NodalTranslationalInertia: Beta parameter should be positive.");

    const Real vel_old = aux_sol_old(_current_node->dof_number(_aux_sys.number(), _vel_num, 0));
    const Real accel_old = aux_sol_old(_current_node->dof_number(_aux_sys.number(), _accel_num, 0));

    const Real accel =
        1. / _beta *
        (((_u[_qp] - _u_old[_qp]) / (_dt * _dt)) - vel_old / _dt - accel_old * (0.5 - _beta));
    const Real vel = vel_old + (_dt * (1 - _gamma)) * accel_old + _gamma * _dt * accel;
    return _mass * (accel + vel * _eta * (1 + _alpha) - _alpha * _eta * vel_old);
  }
}

Real
NodalTranslationalInertia::computeQpJacobian()
{
  mooseAssert(_beta > 0.0, "NodalTranslationalInertia: Beta parameter should be positive.");

  if (_dt == 0)
    return 0;
  else
    return _mass / (_beta * _dt * _dt) + _eta * (1 + _alpha) * _mass * _gamma / _beta / _dt;
}

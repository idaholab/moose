//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewmarkBeta.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", NewmarkBeta);

template <>
InputParameters
validParams<NewmarkBeta>()
{
  InputParameters params = validParams<TimeIntegrator>();
  params.addClassDescription(
      "Computes the first and second time derivative of variable using Newmark-Beta method.");
  params.addParam<Real>("beta", 0.25, "beta value");
  params.addParam<Real>("gamma", 0.5, "gamma value");
  return params;
}

NewmarkBeta::NewmarkBeta(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _u_dot_old(*_sys.solutionUDotOld()),
    _u_dotdot(_sys.solutionUDotDot()),
    _u_dotdot_old(*_sys.solutionUDotDotOld()),
    _du_dotdot_du(_sys.duDotDotDu())
{
}

NewmarkBeta::~NewmarkBeta() {}

void
NewmarkBeta::computeTimeDerivatives()
{
  // compute second derivative
  // according to Newmark-Beta method
  // u_dotdot = first_term - second_term - third_term
  //       first_term = (u - u_old) / beta / dt ^ 2
  //      second_term = u_dot_old / beta / dt
  //       third_term = u_dotdot_old * (1 / 2 / beta - 1)
  _u_dotdot = *_solution;
  _u_dotdot -= _solution_old;
  _u_dotdot *= 1.0 / _beta / _dt / _dt;
  _u_dotdot.add(-1.0 / _beta / _dt, _u_dot_old);
  _u_dotdot.add(-0.5 / _beta + 1.0, _u_dotdot_old);

  // compute first derivative
  // according to Newmark-Beta method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_old * (1 - gamma) * dt
  //       third_term = u_dotdot * gamma * dt
  _u_dot = _u_dot_old;
  _u_dot.add((1.0 - _gamma) * _dt, _u_dotdot_old);
  _u_dot.add(_gamma * _dt, _u_dotdot);

  // make sure _u_dotdot and _u_dot are in good state
  _u_dotdot.close();
  _u_dot.close();

  // used for Jacobian calculations
  _du_dotdot_du = 1.0 / _beta / _dt / _dt;
  _du_dot_du = _gamma / _beta / _dt;
}

void
NewmarkBeta::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

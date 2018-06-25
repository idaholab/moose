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
  params.addParam<Real>("beta", 0.25, "beta value");
  params.addParam<Real>("gamma", 0.5, "gamma value");
  return params;
}

NewmarkBeta::NewmarkBeta(const InputParameters & parameters)
  : TimeIntegrator(parameters), _beta(getParam<Real>("beta")), _gamma(getParam<Real>("gamma"))
{
}

NewmarkBeta::~NewmarkBeta() {}

void
NewmarkBeta::computeTimeDerivatives()
{
  // I think this is done TODO declare _u_dotdot
  // I think this is done TODO declare _du_dotdot_du
  // TODO store _u_dot_old (make _u_dot stateful)
  // TODO store _u_dotdot_old (make _u_dotdot stateful)

  NumericVector<Number> second_term, third_term;
  // compute second derivative
  // according to Newmark-Beta method
  // u_dotdot = first_term - second_term - third_term
  //       first_term = (u - u_old) / beta / dt ^ 2
  //      second_term = u_dot_old / beta / dt
  //       third_term = u_dotdot_old * (1 / 2 / beta - 1)
  _u_dotdot = *_solution;
  _u_dotdot -= _solution_old;
  _u_dotdot *= 1.0 / _beta / _dt / _dt;
  second_term = _u_dot_old;
  second_term *= 1.0 / _beta / _dt;
  third_term = _u_dotdot_old;
  third_term *= 0.5 / _beta - 1.0;
  _u_dotdot -= second_term;
  _u_dotdot -= third_term;

  // compute first derivative
  // according to Newmark-Beta method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_old * (1 - gamma) * dt
  //       third_term = u_dotdot * gamma * dt
  _u_dot = _solution_old;
  second_term = _u_dotdot_old;
  second_term *= (1.0 - _gamma) * _dt;
  third_term = _u_dotdot;
  third_term *= _gamma * _dt;
  _u_dot += second_term;
  _u_dot += third_term;

  // make sure _u_dotdot and _u_dot are in good state
  _u_dotdot.close();
  _u_dot.close();

  // used for Jacobian calculations
  _du_dotdot_du = 1.0 / _beta / _dt / _dt;
  _du_dot_du = _gamma / _beta / dt;
}

void
NewmarkBeta::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

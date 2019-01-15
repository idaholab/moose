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
#include "FEProblemBase.h"

registerMooseObject("MooseApp", NewmarkBeta);

template <>
InputParameters
validParams<NewmarkBeta>()
{
  InputParameters params = validParams<TimeIntegrator>();
  params.addClassDescription(
      "Computes the first and second time derivative of variable using Newmark-Beta method.");
  params.addRangeCheckedParam<Real>("beta", 0.25, "beta > 0.0", "beta value");
  params.addRangeCheckedParam<Real>("gamma", 0.5, "gamma >= 0.5", "gamma value");
  return params;
}

NewmarkBeta::NewmarkBeta(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _du_dotdot_du(_sys.duDotDotDu())
{
  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotRequested(true);
  _fe_problem.setUDotDotOldRequested(true);

  if (_gamma > 2.0 * _beta)
    mooseError("NewmarkBeta: For Newmark method to be unconditionally stable, gamma should lie "
               "between 0.5 and 2.0*beta.");

  if (_gamma != 0.5)
    mooseWarning("NewmarkBeta: For gamma > 0.5, Newmark method is only first order accurate. "
                 "Please use either HHT time integration method or set gamma = 0.5 for second "
                 "order solution accuracy with time.");
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
  if (!_sys.solutionUDot())
    mooseError("NewmarkBeta: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  if (!_sys.solutionUDotDot())
    mooseError("NewmarkBeta: Second time derivative of solution (`u_dotdot`) is not stored. Please "
               "set uDotDotRequested() to true in FEProblemBase befor requesting `u_dotdot`.");

  if (!_sys.solutionUDotOld())
    mooseError("NewmarkBeta: Old time derivative of solution (`u_dot_old`) is not stored. Please "
               "set uDotOldRequested() to true in FEProblemBase befor requesting `u_dot_old`.");

  if (!_sys.solutionUDotDotOld())
    mooseError("NewmarkBeta: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase befor requesting "
               "`u_dotdot_old`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  NumericVector<Number> & u_dotdot = *_sys.solutionUDotDot();
  NumericVector<Number> & u_dot_old = *_sys.solutionUDotOld();
  NumericVector<Number> & u_dotdot_old = *_sys.solutionUDotDotOld();

  u_dotdot = *_solution;
  u_dotdot -= _solution_old;
  u_dotdot *= 1.0 / _beta / _dt / _dt;
  u_dotdot.add(-1.0 / _beta / _dt, u_dot_old);
  u_dotdot.add(-0.5 / _beta + 1.0, u_dotdot_old);

  // compute first derivative
  // according to Newmark-Beta method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_old * (1 - gamma) * dt
  //       third_term = u_dotdot * gamma * dt
  u_dot = u_dot_old;
  u_dot.add((1.0 - _gamma) * _dt, u_dotdot_old);
  u_dot.add(_gamma * _dt, u_dotdot);

  // make sure _u_dotdot and _u_dot are in good state
  u_dotdot.close();
  u_dot.close();

  // used for Jacobian calculations
  _du_dotdot_du = 1.0 / _beta / _dt / _dt;
  _du_dot_du = _gamma / _beta / _dt;
}

void
NewmarkBeta::computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof)
{
  const auto & u_old = _solution_old(dof);
  const auto & u_dot_old = (*_sys.solutionUDotOld())(dof);
  const auto & u_dotdot_old = (*_sys.solutionUDotDotOld())(dof);

  auto u_dotdot = ad_u_dot - u_old;

  u_dotdot *= 1.0 / _beta / _dt / _dt;
  u_dotdot += -1.0 / _beta / _dt * u_dot_old;
  u_dotdot += (-0.5 / _beta + 1.0) * u_dotdot_old;

  // compute first derivative
  // according to Newmark-Beta method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_old * (1 - gamma) * dt
  //       third_term = u_dotdot * gamma * dt
  ad_u_dot = u_dot_old;
  ad_u_dot += ((1.0 - _gamma) * _dt) * u_dotdot_old;
  ad_u_dot += _gamma * _dt * u_dotdot;
}

void
NewmarkBeta::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

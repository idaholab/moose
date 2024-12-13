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

InputParameters
NewmarkBeta::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription(
      "Computes the first and second time derivative of variable using Newmark-Beta method.");
  params.addRangeCheckedParam<Real>("beta", 0.25, "beta > 0.0", "beta value");
  params.addRangeCheckedParam<Real>("gamma", 0.5, "gamma >= 0.5", "gamma value");
  params.addParam<int>("inactive_tsteps",
                       0,
                       "The time derivatives will set to be zero for this number of time steps.");
  return params;
}

NewmarkBeta::NewmarkBeta(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _inactive_tsteps(getParam<int>("inactive_tsteps")),
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

  if (_fe_problem.timeStep() <= _inactive_tsteps)
  {
    u_dot.zero();
    u_dotdot.zero();
  }
  else
  {
    u_dotdot = *_solution;
    computeTimeDerivativeHelper(u_dot, _solution_old, u_dot_old, u_dotdot, u_dotdot_old);
  }

  // make sure _u_dotdot and _u_dot are in good state
  u_dotdot.close();
  u_dot.close();

  // used for Jacobian calculations
  _du_dotdot_du = 1.0 / _beta / _dt / _dt;
  computeDuDotDu();
}

void
NewmarkBeta::computeADTimeDerivatives(ADReal & ad_u_dot,
                                      const dof_id_type & dof,
                                      ADReal & ad_u_dotdot) const
{
  const auto & u_old = _solution_old(dof);
  const auto & u_dot_old = (*_sys.solutionUDotOld())(dof);
  const auto & u_dotdot_old = (*_sys.solutionUDotDotOld())(dof);

  // Seeds ad_u_dotdot with _ad_dof_values and associated derivatives provided via ad_u_dot from
  // MooseVariableData
  ad_u_dotdot = ad_u_dot;

  computeTimeDerivativeHelper(ad_u_dot, u_old, u_dot_old, ad_u_dotdot, u_dotdot_old);
}

void
NewmarkBeta::postResidual(NumericVector<Number> & residual)
{
  residual += *_Re_time;
  residual += *_Re_non_time;
  residual.close();
}

Real
NewmarkBeta::duDotDuCoeff() const
{
  return _gamma / _beta;
}

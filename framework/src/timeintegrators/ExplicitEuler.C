//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitEuler.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", ExplicitEuler);

InputParameters
ExplicitEuler::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription("Time integration using the explicit Euler method.");
  return params;
}

ExplicitEuler::ExplicitEuler(const InputParameters & parameters) : TimeIntegrator(parameters) {}

void
ExplicitEuler::preSolve()
{
  if (_dt == _dt_old)
    _fe_problem.setConstJacobian(true);
  else
    _fe_problem.setConstJacobian(false);
}

void
ExplicitEuler::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ExplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
ExplicitEuler::computeADTimeDerivatives(DualReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        DualReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
ExplicitEuler::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

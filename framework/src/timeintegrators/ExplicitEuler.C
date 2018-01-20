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

template <>
InputParameters
validParams<ExplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

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
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1 / _dt;
  _u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
ExplicitEuler::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrankNicolson.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<CrankNicolson>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

CrankNicolson::CrankNicolson(const InputParameters & parameters)
  : TimeIntegrator(parameters), _residual_old(_nl.addVector("residual_old", false, GHOSTED))
{
}

void
CrankNicolson::computeTimeDerivatives()
{
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 2. / _dt;
  _u_dot.close();

  _du_dot_du = 2. / _dt;
}

void
CrankNicolson::init()
{
  // make sure that time derivative contribution is zero in the first pre-solve step
  _u_dot.zero();
  _u_dot.close();

  _du_dot_du = 0;

  // compute residual for the initial time step
  // Note: we can not directly pass _residual_old in computeResidualType because
  //       the function will call postResidual, which will cause _residual_old
  //       to be added on top of itself prohibited by PETSc.
  _fe_problem.computeResidualType(*_solution, _nl.RHS(), Moose::KT_NONTIME);
  _residual_old = _nl.RHS();
}

void
CrankNicolson::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual += _residual_old;
}

void
CrankNicolson::postStep()
{
  // shift the residual in time
  _residual_old = _Re_non_time;
}

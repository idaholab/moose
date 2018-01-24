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

CrankNicolson::~CrankNicolson() {}

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
CrankNicolson::preSolve()
{
  if (_t_step == 1)
  {
    // make sure that time derivative contribution is zero in the first pre-solve step
    _u_dot.zero();
    _u_dot.close();

    _du_dot_du = 0;

    // for the first time step, compute residual for the old time step
    _fe_problem.computeResidualType(_solution_old, _nl.RHS(), Moose::KT_NONTIME);
    _residual_old = _nl.RHS();
    _residual_old.close();
  }
}

void
CrankNicolson::postStep(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual += _residual_old;
}

void
CrankNicolson::postSolve()
{
  // shift the residual in time
  _residual_old = _Re_non_time;
  _residual_old.close();
}

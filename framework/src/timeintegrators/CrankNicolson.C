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

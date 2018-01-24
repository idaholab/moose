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

ExplicitEuler::~ExplicitEuler() {}

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
ExplicitEuler::postStep(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}

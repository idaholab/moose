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

#include "RungeKutta2.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

template<>
InputParameters validParams<RungeKutta2>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

RungeKutta2::RungeKutta2(const std::string & name, InputParameters parameters) :
    TimeIntegrator(name, parameters),
    _stage(0)
{
  _fe_problem.setConstJacobian(true);
}

RungeKutta2::~RungeKutta2()
{
}

void
RungeKutta2::preSolve()
{
}

void
RungeKutta2::computeTimeDerivatives()
{
  _u_dot  = *_nl.currentSolution();
  if (_stage == 1)
  {
    _u_dot -= _nl.solutionOld();
    _u_dot *= 2. / _dt;

    _du_dot_du = 2. / _dt;
  }
  else
  {
    _u_dot -= _nl.solutionOlder();
    _u_dot *= 1. / _dt;

    _du_dot_du = 1. / _dt;
  }
  _u_dot.close();
  _du_dot_du.close();
}

void
RungeKutta2::solve()
{
  Real time = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_half = (time + time_old) / 2.;

  _stage = 1;
  _fe_problem.time() = time_half;
  _fe_problem.getNonlinearSystem().sys().solve();

  _fe_problem.copyOldSolutions();

  // ---------------------------------
  std::cout << " 2. stage" << std::endl;
  _stage = 2;
  _fe_problem.time() = time;
  _fe_problem.timeOld() = time_half;
  _fe_problem.getNonlinearSystem().sys().solve();
}

void
RungeKutta2::postStep(NumericVector<Number> & residual)
{
  residual += _Re_non_time;
  residual += _Re_time;
  residual.close();
}

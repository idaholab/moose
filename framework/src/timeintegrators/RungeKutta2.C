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
    _sln_half(_nl.addVector("sln_half", true, GHOSTED)),
    _Re_half(_nl.addVector("Re_half", true, GHOSTED)),
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
  _u_dot -= _nl.solutionOld();
  _u_dot *= 1. / _dt;
  _u_dot.close();

  _du_dot_du = 1. / _dt;
  _du_dot_du.close();
}

void
RungeKutta2::solve()
{
  Real time = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_half = (time + time_old) / 2.;

  _stage = 1;
  // make sure that time derivative contribution is zero in the first pre-solve step
  _u_dot.zero();
  _u_dot.close();
  _du_dot_du.zero();
  _du_dot_du.close();

  _fe_problem.time() = time_half;
  _nl.sys().solve();

  // ---------------------------------
  std::cout << " 2. stage" << std::endl;
  _stage = 2;
  _fe_problem.time() = time;
  _fe_problem.timeOld() = time_half;
  _nl.sys().solve();
}

void
RungeKutta2::postStep(NumericVector<Number> & residual)
{
  residual += _Re_non_time;
  if (_stage == 1)
    residual *= 0.5;
  residual += _Re_time;
  residual.close();
}

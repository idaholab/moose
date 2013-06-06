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

template<>
InputParameters validParams<ExplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

ExplicitEuler::ExplicitEuler(const std::string & name, InputParameters parameters) :
    TimeIntegrator(name, parameters),
    _Re_old(_nl.addVector("Re_old", true, GHOSTED))
{
}

ExplicitEuler::~ExplicitEuler()
{
}

void
ExplicitEuler::preSolve()
{
  // make sure that time derivative contribution is zero in the first pre-solve step
  _u_dot.zero();
  _u_dot.close();

  _du_dot_du.zero();
  _du_dot_du.close();

  // evaluate non-time kernels and save off the residual
  _fe_problem.computeResidualType(_nl.solutionOld(), *_nl.sys().rhs, Moose::KT_NONTIME);
  _Re_old = _Re_non_time;
  _Re_old.close();
}

void
ExplicitEuler::computeTimeDerivatives()
{
  _u_dot  = *_nl.currentSolution();
  _u_dot -= _nl.solutionOld();
  _u_dot *= 1 / _dt;
  _u_dot.close();

  _du_dot_du = 1.0 / _dt;
  _du_dot_du.close();
}

void
ExplicitEuler::solve()
{
  // make sure we evaluate only-time kernels, non-time ones are constant over the time step
  // and we precalculated those in preSolve()
  _fe_problem.setKernelTypeResidual(Moose::KT_TIME);
  TimeIntegrator::solve();
}

void
ExplicitEuler::postStep(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_old;
  residual.close();
}


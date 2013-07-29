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
    TimeIntegrator(name, parameters)
{
}

ExplicitEuler::~ExplicitEuler()
{
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
ExplicitEuler::postStep(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}


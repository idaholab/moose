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

#include "SteadyState.h"

template<>
InputParameters validParams<SteadyState>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

SteadyState::SteadyState(const std::string & name, InputParameters parameters) :
    TimeIntegrator(name, parameters)
{
}

SteadyState::~SteadyState()
{
}

void
SteadyState::computeTimeDerivatives()
{
  _u_dot.zero();
  _u_dot.close();

  _du_dot_du.zero();
  _du_dot_du.close();
}

void
SteadyState::postStep(NumericVector<Number> & residual)
{
  residual += _Re_non_time;
  residual.close();
}

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

// libMesh includes
#include "libmesh/numeric_vector.h"

template <>
InputParameters
validParams<SteadyState>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

SteadyState::SteadyState(const InputParameters & parameters) : TimeIntegrator(parameters) {}

SteadyState::~SteadyState() {}

void
SteadyState::computeTimeDerivatives()
{
  _u_dot.zero();
  _u_dot.close();

  _du_dot_du = 0;
}

void
SteadyState::postStep(NumericVector<Number> & residual)
{
  residual += _Re_non_time;
  residual.close();
}

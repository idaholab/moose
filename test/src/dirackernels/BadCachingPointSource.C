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

#include "BadCachingPointSource.h"

template <>
InputParameters
validParams<BadCachingPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  return params;
}

BadCachingPointSource::BadCachingPointSource(const InputParameters & parameters)
  : DiracKernel(parameters), _called(0)
{
}

void
BadCachingPointSource::addPoints()
{
  // Add points on the unit square using user-defined IDs.  The first
  // time through a PointLocator will look up their elements, but on
  // subsequent calls to addPoints(), it should used cached values.
  addPoint(Point(.25, .25), 0 + _called);
  addPoint(Point(.75, .25), 1 + _called);
  addPoint(Point(.75, .75), 2 + _called);
  addPoint(Point(.25, .75), 3 + _called);

  _called++;
}

Real
BadCachingPointSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been
  // brought over to the left side.  The value of the forcing is 1.0 each time
  return -_test[_i][_qp] * 1.0;
}

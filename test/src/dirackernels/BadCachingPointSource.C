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

template<>
InputParameters validParams<BadCachingPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  return params;
}

BadCachingPointSource::BadCachingPointSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters)
{
}

void
BadCachingPointSource::addPoints()
{
  // Gets incremented every time this function is called.  Simulates a user
  // adding points with a 'unique' ID that is already used by another point.
  static unsigned called = 0;

  // Add points on the unit square using user-defined IDs.  The first
  // time through a PointLocator will look up their elements, but on
  // subsequent calls to addPoints(), it should used cached values.
  addPoint(Point(.25, .25), 0 + called);
  addPoint(Point(.75, .25), 1 + called);
  addPoint(Point(.75, .75), 2 + called);
  addPoint(Point(.25, .75), 3 + called);

  called++;
}

Real
BadCachingPointSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been
  // brought over to the left side.  The value of the forcing is 1.0 each time
  return -_test[_i][_qp] * 1.0;
}


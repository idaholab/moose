//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BadCachingPointSource.h"

registerMooseObject("MooseTestApp", BadCachingPointSource);

InputParameters
BadCachingPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
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

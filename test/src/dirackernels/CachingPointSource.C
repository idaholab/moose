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

#include "CachingPointSource.h"

template <>
InputParameters
validParams<CachingPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  return params;
}

CachingPointSource::CachingPointSource(const InputParameters & parameters) : DiracKernel(parameters)
{
}

void
CachingPointSource::addPoints()
{
  // Add points on the unit square using user-defined IDs.  The first
  // time through a PointLocator will look up their elements, but on
  // subsequent calls to addPoints(), it should used cached values.
  Real eps = 1.e-3;
  addPoint(Point(.25 + eps, .25 + eps), 0);
  addPoint(Point(.75 + eps, .25 + eps), 1);
  addPoint(Point(.75 + eps, .75 + eps), 2);
  addPoint(Point(.25 + eps, .75 + eps), 3);
}

Real
CachingPointSource::computeQpResidual()
{
  // Grab the user-defined ID for the Dirac point we are currently on.
  unsigned id = currentPointCachedID();

  // If looking up the ID failed, we can't continue.
  if (id == libMesh::invalid_uint)
    mooseError("User id for point ", _current_point, " is ", id);

  // This is negative because it's a forcing function that has been
  // brought over to the left side.  The value of the forcing is equal
  // to the ID of the point in this simple example.
  return -_test[_i][_qp] * static_cast<Real>(id);
}

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

#ifndef NEARESTPOINTLAYEREDAVERAGE_H
#define NEARESTPOINTLAYEREDAVERAGE_H

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "NearestPointBase.h"
#include "LayeredAverage.h"

// Forward Declarations
class NearestPointLayeredAverage;

template <>
InputParameters validParams<NearestPointLayeredAverage>();

/**
 * This UserObject computes averages of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered average
 * closest to each one of those points.
 */
class NearestPointLayeredAverage : public NearestPointBase<LayeredAverage>
{
public:
  NearestPointLayeredAverage(const InputParameters & parameters);
};

#endif

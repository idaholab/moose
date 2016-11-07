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

// MOOSE includes
#include "NearestPointLayeredAverage.h"
#include "LayeredAverage.h"

template <>
InputParameters
validParams<NearestPointLayeredAverage>()
{
  InputParameters params = nearestPointBaseValidParams<LayeredAverage>();

  return params;
}

NearestPointLayeredAverage::NearestPointLayeredAverage(const InputParameters & parameters)
  : NearestPointBase<LayeredAverage>(parameters)
{
}

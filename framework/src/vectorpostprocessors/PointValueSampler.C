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

#include "PointValueSampler.h"

template <>
InputParameters
validParams<PointValueSampler>()
{
  InputParameters params = validParams<PointSamplerBase>();

  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

PointValueSampler::PointValueSampler(const InputParameters & parameters)
  : PointSamplerBase(parameters)
{
  _points = getParam<std::vector<Point>>("points");

  _ids.resize(_points.size());

  for (unsigned int i = 0; i < _points.size(); i++)
    _ids[i] = i;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

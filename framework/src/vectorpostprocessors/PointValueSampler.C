//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointValueSampler.h"

#include <numeric>

registerMooseObject("MooseApp", PointValueSampler);

InputParameters
PointValueSampler::validParams()
{
  InputParameters params = PointSamplerBase::validParams();
  params.addClassDescription("Sample a variable at specific points.");
  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

PointValueSampler::PointValueSampler(const InputParameters & parameters)
  : PointSamplerBase(parameters)
{
  _points = getParam<std::vector<Point>>("points");
}

void
PointValueSampler::initialize()
{
  // Generate new Ids if the point vector has grown (non-negative counting numbers)
  if (_points.size() > _ids.size())
  {
    auto old_size = _ids.size();
    _ids.resize(_points.size());
    std::iota(_ids.begin() + old_size, _ids.end(), old_size);
  }
  // Otherwise sync the ids array to be smaller if the point vector has been shrunk
  else if (_points.size() < _ids.size())
    _ids.resize(_points.size());

  PointSamplerBase::initialize();
}

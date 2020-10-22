//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeasuredDataPointSampler.h"

#include <numeric>

registerMooseObject("MooseApp", MeasuredDataPointSampler);

defineLegacyParams(MeasuredDataPointSampler);

// FIXME LYNN  This is an exact copy of PointSampler because I need to add measurement data to the
// values so that it can be sorted with the values.  This is all because SamplerBase has protected
// inheritance in PointSamplerBase.

InputParameters
MeasuredDataPointSampler::validParams()
{
  InputParameters params = MeasuredDataPointSamplerBase::validParams();

  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

MeasuredDataPointSampler::MeasuredDataPointSampler(const InputParameters & parameters)
  : MeasuredDataPointSamplerBase(parameters)
{
  _points = getParam<std::vector<Point>>("points");
}

void
MeasuredDataPointSampler::initialize()
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

  MeasuredDataPointSamplerBase::initialize();
}

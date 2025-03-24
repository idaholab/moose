//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = PointVariableSamplerBase::validParams();
  params.addClassDescription("Sample a variable at specific points.");
  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

PointValueSampler::PointValueSampler(const InputParameters & parameters)
  : PointVariableSamplerBase(parameters)
{
  _points = getParam<std::vector<Point>>("points");
}

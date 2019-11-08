//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicPointValueSampler.h"

registerMooseObject("MooseTestApp", DynamicPointValueSampler);

InputParameters
DynamicPointValueSampler::validParams()
{
  InputParameters params = LineValueSampler::validParams();

  params.addParam<unsigned int>(
      "num_points_adder", 2, "The number of new points to add each iteration");

  params.addParam<bool>(
      "test_transfer_points_vector",
      false,
      "Set to true to use the transferPointsVector API (Default setPointsVector).");

  return params;
}

DynamicPointValueSampler::DynamicPointValueSampler(const InputParameters & parameters)
  : LineValueSampler(parameters),
    _adder(getParam<unsigned int>("num_points_adder")),
    _use_transfer(getParam<bool>("test_transfer_points_vector"))
{
}

void
DynamicPointValueSampler::initialize()
{
  _num_points = _num_points + _adder;

  std::vector<Point> points;
  generatePointsAndIDs(_start_point, _end_point, _num_points, points, _ids);

  // We don't need to use the public API here, but we are doing it for testing
  if (_use_transfer)
    transferPointsVector(std::move(points));
  else
    setPointsVector(points);

  LineValueSampler::initialize();
}

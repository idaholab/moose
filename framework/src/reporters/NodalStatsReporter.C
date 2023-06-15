//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalStatsReporter.h"

#include "NodalReporter.h"
#include <string>

InputParameters
NodalStatsReporter::validParams()
{
  InputParameters params = NodalReporter::validParams();
  params.addParam<std::string>("base_name", "Name to append to reporters.");
  return params;
}

NodalStatsReporter::NodalStatsReporter(const InputParameters & parameters)
  : NodalReporter(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _max(declareValueByName<Real>(_base_name + "max")),
    _min(declareValueByName<Real>(_base_name + "min")),
    _average(declareValueByName<Real>(_base_name + "average")),
    _number_nodes(declareValueByName<int>(_base_name + "number_nodes"))
{
}
void
NodalStatsReporter::initialize()
{
  _max = std::numeric_limits<Real>::min();
  _min = std::numeric_limits<Real>::max();
  _average = 0;
  _number_nodes = 0;
}

void
NodalStatsReporter::execute()
{
  // Get value to to update statistics
  Real value = computeValue();

  if (_max < value)
    _max = value;

  if (_min > value)
    _min = value;

  // Update the total and the number to get the average finalize
  _average += value;
  _number_nodes++;
}
void
NodalStatsReporter::threadJoin(const UserObject & uo)
{
  const NodalStatsReporter & es = static_cast<const NodalStatsReporter &>(uo);
  if (_max < es._max)
    _max = es._max;

  if (_min > es._min)
    _min = es._min;

  _average += es._average;
  _number_nodes += es._number_nodes;
}
void
NodalStatsReporter::finalize()
{
  _communicator.max(_max);
  _communicator.min(_min);
  _communicator.sum(_average);
  _communicator.sum(_number_nodes);

  // Compute the average;
  _average /= _number_nodes;
}

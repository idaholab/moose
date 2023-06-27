//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementStatistics.h"

InputParameters
ElementStatistics::validParams()
{
  InputParameters params = ElementReporter::validParams();
  params.addParam<std::string>("base_name", "Name to append to reporters.");
  return params;
}

ElementStatistics::ElementStatistics(const InputParameters & parameters)
  : ElementReporter(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _max(declareValueByName<Real>(_base_name + "max")),
    _min(declareValueByName<Real>(_base_name + "min")),
    _average(declareValueByName<Real>(_base_name + "average")),
    _integral(declareValueByName<Real>(_base_name + "integral")),
    _number_elements(declareValueByName<int>(_base_name + "number_elements"))
{
}

void
ElementStatistics::initialize()
{
  _max = std::numeric_limits<Real>::min();
  _min = std::numeric_limits<Real>::max();
  _average = 0;
  _integral = 0;
  _number_elements = 0;
}

void
ElementStatistics::execute()
{
  // Get value to to update statistics
  Real value = computeValue();

  if (_max < value)
    _max = value;

  if (_min > value)
    _min = value;

  _integral += value * _current_elem_volume;

  // Update the total and the number to get the average when "finalizing"
  _average += value;
  _number_elements++;
}

void
ElementStatistics::threadJoin(const UserObject & uo)
{
  const ElementStatistics & ele_uo = static_cast<const ElementStatistics &>(uo);
  _max = std::max(_max, ele_uo._max);
  _min = std::min(_min, ele_uo._min);
  _integral += ele_uo._integral;
  _average += ele_uo._average;
  _number_elements += ele_uo._number_elements;
}

void
ElementStatistics::finalize()
{
  _communicator.max(_max);
  _communicator.min(_min);
  _communicator.sum(_integral);
  _communicator.sum(_average);
  _communicator.sum(_number_elements);

  // Compute the average;
  _average /= _number_elements;
}

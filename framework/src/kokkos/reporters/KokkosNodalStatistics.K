//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalStatistics.h"

InputParameters
KokkosNodalStatistics::validParams()
{
  InputParameters params = NodalReporter::validParams();
  params.addParam<std::string>("base_name", "Name to append to reporters.");
  return params;
}

KokkosNodalStatistics::KokkosNodalStatistics(const InputParameters & parameters)
  : NodalReporter(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _max(declareValueByName<Real>(_base_name + "max")),
    _min(declareValueByName<Real>(_base_name + "min")),
    _average(declareValueByName<Real>(_base_name + "average")),
    _number_nodes(declareValueByName<unsigned int>(_base_name + "number_nodes"))
{
}

void
KokkosNodalStatistics::initialize()
{
  _max = std::numeric_limits<Real>::min();
  _min = std::numeric_limits<Real>::max();
  _average = 0;
  _number_nodes = 0;

  allocateReductionBuffer(4);
}

void
KokkosNodalStatistics::finalize()
{
  _max = _reduction_buffer[0];
  _min = _reduction_buffer[1];
  _average = _reduction_buffer[2];
  _number_nodes = _reduction_buffer[3];

  _communicator.max(_max);
  _communicator.min(_min);
  _communicator.sum(_average);
  _communicator.sum(_number_nodes);

  // Compute the average;
  _average /= _number_nodes;
}

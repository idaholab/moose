//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <numeric>
#include "StatVector.h"

registerMooseObject("MooseTestApp", StatVector);

InputParameters
StatVector::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum stat("sum min max");
  params.addParam<MooseEnum>("stat", stat, "Statistic to compute.");
  params.addRequiredParam<VectorPostprocessorName>("object", "Object name.");
  params.addRequiredParam<std::string>("vector", "Vector name.");
  return params;
}

StatVector::StatVector(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _stat(getParam<MooseEnum>("stat")),
    _vpp(getVectorPostprocessorValue("object", getParam<std::string>("vector")))
{
}

void
StatVector::execute()
{
  if (_stat == "sum")
    _value = std::accumulate(_vpp.begin(), _vpp.end(), 0.);
  else if (_stat == "min")
    _value = _vpp.empty() ? 0. : *std::min_element(_vpp.begin(), _vpp.end());
  else if (_stat == "max")
    _value = _vpp.empty() ? 0. : *std::max_element(_vpp.begin(), _vpp.end());
}

Real
StatVector::getValue()
{
  return _value;
}

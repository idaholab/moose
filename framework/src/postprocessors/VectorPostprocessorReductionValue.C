//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorReductionValue.h"
#include "VectorPostprocessorInterface.h"
#include <limits>

registerMooseObject("MooseApp", VectorPostprocessorReductionValue);

InputParameters
VectorPostprocessorReductionValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor", "The vectorpostprocessor from which a value is extracted");
  params.addRequiredParam<std::string>("vector_name",
                                       "Name of the vector for which to report a value");
  params.addParam<MooseEnum>(
      "value_type",
      MooseEnum("max=0 min=1 sum=2 average=3", "sum"),
      "Type of reduction operation. Options are max, min, sum, and average.");
  params.addClassDescription("Takes a VectorPostprocessor and performs a reduction operation on it "
                             "(max, min, sum, average) and stores as postprocessor.");

  return params;
}

VectorPostprocessorReductionValue::VectorPostprocessorReductionValue(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vectorpostprocessor")),
    _vector_name(getParam<std::string>("vector_name")),
    _vpp_values(getVectorPostprocessorValue("vectorpostprocessor", _vector_name)),
    _value_type(getParam<MooseEnum>("value_type"))

{
}

Real
VectorPostprocessorReductionValue::getValue()
{
  Real r = 0;
  switch (_value_type)
  {
    case 0:
      r = std::numeric_limits<Real>::lowest();
      for (auto & e : _vpp_values)
        if (e > r)
          r = e;
      break;

    case 1:
      r = std::numeric_limits<Real>::max();
      for (auto & e : _vpp_values)
        if (e < r)
          r = e;
      break;

    case 2:
      r = 0;
      for (auto & e : _vpp_values)
        r += e;
      break;

    case 3:
      r = 0;
      for (auto & e : _vpp_values)
        r += e / _vpp_values.size();
      return r;
  }
  return r;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalePostprocessor.h"

template <>
InputParameters
validParams<ScalePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("value", "The postprocessor to be scaled");
  params.addParam<Real>("scaling_factor", 1.0, "The scaling factor");
  return params;
}

ScalePostprocessor::ScalePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(getPostprocessorValue("value")),
    _scaling_factor(getParam<Real>("scaling_factor"))
{
}

PostprocessorValue
ScalePostprocessor::getValue()
{
  return _scaling_factor * _value;
}

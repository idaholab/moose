//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RelativeDifferencePostprocessor.h"

registerMooseObject("MooseApp", RelativeDifferencePostprocessor);

InputParameters
RelativeDifferencePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>("value1", "First post-processor");
  params.addRequiredParam<PostprocessorName>("value2",
                                             "Second post-processor, base for relative difference");

  params.addClassDescription("Computes the absolute value of the relative "
                             "difference between 2 post-processor values.");

  return params;
}

RelativeDifferencePostprocessor::RelativeDifferencePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value1(getPostprocessorValue("value1")),
    _value2(getPostprocessorValue("value2"))
{
}

void
RelativeDifferencePostprocessor::initialize()
{
}

void
RelativeDifferencePostprocessor::execute()
{
}

PostprocessorValue
RelativeDifferencePostprocessor::getValue()
{
  if (MooseUtils::absoluteFuzzyEqual(_value2, 0))
    return std::abs(_value1 - _value2);
  else
    return std::abs((_value1 - _value2) / _value2);
}

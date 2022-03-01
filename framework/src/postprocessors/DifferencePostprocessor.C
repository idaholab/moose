//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DifferencePostprocessor.h"

registerMooseObject("MooseApp", DifferencePostprocessor);

InputParameters
DifferencePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>("value1", "First value");
  params.addRequiredParam<PostprocessorName>("value2", "Second value");
  params.addClassDescription("Computes the difference between two postprocessors");
  return params;
}

DifferencePostprocessor::DifferencePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value1(getPostprocessorValue("value1")),
    _value2(getPostprocessorValue("value2"))
{
}

void
DifferencePostprocessor::initialize()
{
}

void
DifferencePostprocessor::execute()
{
}

PostprocessorValue
DifferencePostprocessor::getValue()
{
  return _value1 - _value2;
}

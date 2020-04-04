//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorComparison.h"

registerMooseObject("MooseApp", PostprocessorComparison);

InputParameters
PostprocessorComparison::validParams()
{
  InputParameters params = ComparisonPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>("value_a",
                                             "The first post-processor or value in the comparison");
  params.addRequiredParam<PostprocessorName>(
      "value_b", "The second post-processor or value in the comparison");

  params.addClassDescription("Compares two post-processors and produces a boolean value");

  return params;
}

PostprocessorComparison::PostprocessorComparison(const InputParameters & parameters)
  : ComparisonPostprocessor(parameters),

    _value_a(getPostprocessorValue("value_a")),
    _value_b(getPostprocessorValue("value_b")),

    _comparison_value(0.0)
{
}

void
PostprocessorComparison::initialize()
{
  _comparison_value = 0.0;
}

void
PostprocessorComparison::execute()
{
  _comparison_value = comparisonIsTrue(_value_a, _value_b);
}

PostprocessorValue
PostprocessorComparison::getValue()
{
  return _comparison_value;
}

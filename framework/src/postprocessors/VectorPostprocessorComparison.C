//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorComparison.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("MooseApp", VectorPostprocessorComparison);

InputParameters
VectorPostprocessorComparison::validParams()
{
  InputParameters params = ComparisonPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_a", "The first vector post-processor in the comparison");
  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_b", "The second vector post-processor in the comparison");
  params.addRequiredParam<std::string>(
      "vector_name_a", "The name of the vector in the first vector post-processor to compare");
  params.addRequiredParam<std::string>(
      "vector_name_b", "The name of the vector in the second vector post-processor to compare");

  params.addClassDescription(
      "Compares two vector post-processors of equal size and produces a boolean value");

  return params;
}

VectorPostprocessorComparison::VectorPostprocessorComparison(const InputParameters & parameters)
  : ComparisonPostprocessor(parameters),

    _values_a(getVectorPostprocessorValue("vectorpostprocessor_a",
                                          getParam<std::string>("vector_name_a"))),
    _values_b(getVectorPostprocessorValue("vectorpostprocessor_b",
                                          getParam<std::string>("vector_name_b"))),

    _comparison_value(0.0)
{
}

void
VectorPostprocessorComparison::initialize()
{
  _comparison_value = 0.0;
}

void
VectorPostprocessorComparison::execute()
{
  if (_values_a.size() != _values_b.size())
    mooseError("The compared vector post-processors must have the same size");

  // Set comparison value to "false" if comparison is false for any pair of elements
  bool comparison_bool = true;
  for (unsigned int i = 0; i < _values_a.size(); ++i)
    comparison_bool = comparison_bool && comparisonIsTrue(_values_a[i], _values_b[i]);

  _comparison_value = comparison_bool;
}

PostprocessorValue
VectorPostprocessorComparison::getValue()
{
  return _comparison_value;
}

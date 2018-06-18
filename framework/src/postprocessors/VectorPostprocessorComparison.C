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

template <>
InputParameters
validParams<VectorPostprocessorComparison>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_a", "The first vector post-processor in the comparison");
  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_b", "The second vector post-processor in the comparison");
  params.addRequiredParam<std::string>(
      "vector_name_a", "The name of the vector in the first vector post-processor to compare");
  params.addRequiredParam<std::string>(
      "vector_name_b", "The name of the vector in the second vector post-processor to compare");

  MooseEnum comparison_type("equals greater_than_equals less_than_equals greater_than less_than");
  params.addRequiredParam<MooseEnum>("comparison_type",
                                     comparison_type,
                                     "The type of comparison to perform. Options are: " +
                                         comparison_type.getRawNames());

  params.addParam<Real>("absolute_tolerance",
                        libMesh::TOLERANCE * libMesh::TOLERANCE,
                        "Absolute tolerance used in comparisons");

  params.addClassDescription(
      "Compares two vector post-processors of equal size and produces a boolean value");

  return params;
}

VectorPostprocessorComparison::VectorPostprocessorComparison(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _values_a(getVectorPostprocessorValue("vectorpostprocessor_a",
                                          getParam<std::string>("vector_name_a"))),
    _values_b(getVectorPostprocessorValue("vectorpostprocessor_b",
                                          getParam<std::string>("vector_name_b"))),

    _comparison_type(getParam<MooseEnum>("comparison_type")),

    _absolute_tolerance(getParam<Real>("absolute_tolerance")),

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

  // Initialize comparison value to "true"
  _comparison_value = 1.0;

  // Set comparison value to "false" if comparison is false for any pair of elements
  for (unsigned int i = 0; i < _values_a.size(); ++i)
    if (_comparison_type == "equals")
    {
      if (!MooseUtils::absoluteFuzzyEqual(_values_a[i], _values_b[i], _absolute_tolerance))
        _comparison_value = -1.0;
    }
    else if (_comparison_type == "greater_than_equals")
    {
      if (!MooseUtils::absoluteFuzzyGreaterEqual(_values_a[i], _values_b[i], _absolute_tolerance))
        _comparison_value = -1.0;
    }
    else if (_comparison_type == "less_than_equals")
    {
      if (!MooseUtils::absoluteFuzzyLessEqual(_values_a[i], _values_b[i], _absolute_tolerance))
        _comparison_value = -1.0;
    }
    else if (_comparison_type == "greater_than")
    {
      if (!MooseUtils::absoluteFuzzyGreaterThan(_values_a[i], _values_b[i], _absolute_tolerance))
        _comparison_value = -1.0;
    }
    else if (_comparison_type == "less_than")
    {
      if (!MooseUtils::absoluteFuzzyLessThan(_values_a[i], _values_b[i], _absolute_tolerance))
        _comparison_value = -1.0;
    }
}

PostprocessorValue
VectorPostprocessorComparison::getValue()
{
  return _comparison_value;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComparisonPostprocessor.h"

InputParameters
ComparisonPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum comparison_type("equals greater_than_equals less_than_equals greater_than less_than");
  params.addRequiredParam<MooseEnum>("comparison_type",
                                     comparison_type,
                                     "The type of comparison to perform. Options are: " +
                                         comparison_type.getRawNames());

  params.addParam<Real>("absolute_tolerance",
                        libMesh::TOLERANCE * libMesh::TOLERANCE,
                        "Absolute tolerance used in comparisons");

  return params;
}

ComparisonPostprocessor::ComparisonPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _comparison_type(getParam<MooseEnum>("comparison_type").getEnum<ComparisonType>()),
    _absolute_tolerance(getParam<Real>("absolute_tolerance"))
{
}

bool
ComparisonPostprocessor::comparisonIsTrue(const Real & a, const Real & b) const
{
  switch (_comparison_type)
  {
    case ComparisonType::EQUALS:
      return MooseUtils::absoluteFuzzyEqual(a, b, _absolute_tolerance);

    case ComparisonType::GREATER_THAN_EQUALS:
      return MooseUtils::absoluteFuzzyGreaterEqual(a, b, _absolute_tolerance);

    case ComparisonType::LESS_THAN_EQUALS:
      return MooseUtils::absoluteFuzzyLessEqual(a, b, _absolute_tolerance);

    case ComparisonType::GREATER_THAN:
      return MooseUtils::absoluteFuzzyGreaterThan(a, b, _absolute_tolerance);

    case ComparisonType::LESS_THAN:
      return MooseUtils::absoluteFuzzyLessThan(a, b, _absolute_tolerance);

    default:
      mooseError("Invalid comparison type.");
  }
}

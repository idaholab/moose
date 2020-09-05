//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorDifferenceMeasure.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("MooseApp", VectorPostprocessorDifferenceMeasure);

defineLegacyParams(VectorPostprocessorDifferenceMeasure);

InputParameters
VectorPostprocessorDifferenceMeasure::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_a", "The first vector post-processor in the comparison");
  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_b", "The second vector post-processor in the comparison");
  params.addRequiredParam<std::string>(
      "vector_name_a", "The name of the vector in the first vector post-processor to compare");
  params.addRequiredParam<std::string>(
      "vector_name_b", "The name of the vector in the second vector post-processor to compare");
  MooseEnum difference_type("l2 difference");
  params.addRequiredParam<MooseEnum>("difference_type",
                                     difference_type,
                                     "The type of differencing to perform. Options are: " +
                                         difference_type.getRawNames());
  params.addClassDescription(
      "Compares two vector post-processors of equal size and produces a measure of their distance");

  return params;
}

/**
 * Performs the selected difference on the two values
 *
 * @param[in] a   First value in the difference
 * @param[in] b   Second value in the difference
 *
 * @return Real value for the difference
 */
VectorPostprocessorDifferenceMeasure::VectorPostprocessorDifferenceMeasure(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _values_a(getVectorPostprocessorValue("vectorpostprocessor_a",
                                          getParam<std::string>("vector_name_a"))),
    _values_b(getVectorPostprocessorValue("vectorpostprocessor_b",
                                          getParam<std::string>("vector_name_b"))),
    _difference_type(getParam<MooseEnum>("difference_type").getEnum<DifferenceType>()),
    _difference_value(0.0)
{
}

void
VectorPostprocessorDifferenceMeasure::initialize()
{
  _difference_value = 0.0;
}

void
VectorPostprocessorDifferenceMeasure::execute()
{
  errorCheck();
  switch (_difference_type)
  {
    case DifferenceType::L2:
      return computeL2();
      break;
    case DifferenceType::DIFFERENCE:
      return computeDifference();
      break;
    default:
      mooseError("Invalid difference type.");
  }
}

PostprocessorValue
VectorPostprocessorDifferenceMeasure::getValue()
{
  return _difference_value;
}

void
VectorPostprocessorDifferenceMeasure::computeL2()
{
  Real diff = 0.0;
  for (unsigned int i = 0; i < _values_a.size(); ++i)
  {
    diff += (_values_a[i] - _values_b[i]) * (_values_a[i] - _values_b[i]);
  }
  _difference_value = std::sqrt(diff);
}

void
VectorPostprocessorDifferenceMeasure::computeDifference()
{
  Real diff = 0.0;
  for (unsigned int i = 0; i < _values_a.size(); ++i)
  {
    diff += (_values_a[i] - _values_b[i]);
  }
  _difference_value = diff;
}

void
VectorPostprocessorDifferenceMeasure::errorCheck()
{
  if (_values_a.size() == 0)
    mooseError("vectorpostprocessor_a in VectorProcessorDifferenceMeasure is empty.\n"
               "Ensure that data has been written to this post-processor at this\n"
               "point in the computation and that the names are correct.");
  else if (_values_b.size() == 0)
    mooseError("vectorpostprocessor_a in VectorProcessorDifferenceMeasure is empty.\n"
               "Ensure that data has been written to this post-processor at this\n"
               "point in the computation and that the names are correct.");
  else if (_values_a.size() != _values_b.size())
    mooseError("The VectorPostprocessorDifferenceMeasure must have the same sized vectors."
               "\n   vectorpostprocessor_a is size: ",
               _values_a.size(),
               "\n   vectorpostprocessor_b is size: ",
               _values_b.size());
}

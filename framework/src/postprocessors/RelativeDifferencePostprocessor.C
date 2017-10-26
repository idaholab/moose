/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "RelativeDifferencePostprocessor.h"

template <>
InputParameters
validParams<RelativeDifferencePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

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

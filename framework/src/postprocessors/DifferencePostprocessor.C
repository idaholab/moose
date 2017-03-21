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
#include "DifferencePostprocessor.h"

template <>
InputParameters
validParams<DifferencePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("value1", "First value");
  params.addRequiredParam<PostprocessorName>("value2", "Second value");

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

void
DifferencePostprocessor::threadJoin(const UserObject & /*uo*/)
{
  // nothing to do here, general PPS do not run threaded
}

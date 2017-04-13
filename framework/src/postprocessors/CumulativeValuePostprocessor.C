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

#include "CumulativeValuePostprocessor.h"

template <>
InputParameters
validParams<CumulativeValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  return params;
}

CumulativeValuePostprocessor::CumulativeValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _sum(0.0),
    _sum_old(getPostprocessorValueOldByName(name())),
    _pps_value(getPostprocessorValue("postprocessor"))
{
}

void
CumulativeValuePostprocessor::initialize()
{
}

void
CumulativeValuePostprocessor::execute()
{
  _sum = _sum_old + _pps_value;
}

Real
CumulativeValuePostprocessor::getValue()
{
  return _sum;
}

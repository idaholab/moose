//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CumulativeValuePostprocessor.h"

registerMooseObject("MooseApp", CumulativeValuePostprocessor);

InputParameters
CumulativeValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Creates a cumulative sum of a Postprocessor value with time.");
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

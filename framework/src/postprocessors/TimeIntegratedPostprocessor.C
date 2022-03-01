//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegratedPostprocessor.h"

registerMooseObject("MooseApp", TimeIntegratedPostprocessor);
registerMooseObjectRenamed("MooseApp",
                           TotalVariableValue,
                           "04/01/2022 00:00",
                           TimeIntegratedPostprocessor);

InputParameters
TimeIntegratedPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Integrate a Postprocessor value over time using trapezoidal rule.");
  params.addParam<PostprocessorName>("value", "The name of the postprocessor");
  return params;
}

TimeIntegratedPostprocessor::TimeIntegratedPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(0),
    _value_old(getPostprocessorValueOldByName(name())),
    _pps_value(getPostprocessorValue("value")),
    _pps_value_old(getPostprocessorValueOld("value"))
{
}

void
TimeIntegratedPostprocessor::initialize()
{
}

void
TimeIntegratedPostprocessor::execute()
{
  _value = _value_old + 0.5 * (_pps_value + _pps_value_old) * _dt;
}

Real
TimeIntegratedPostprocessor::getValue()
{
  return _value;
}

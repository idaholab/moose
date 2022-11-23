//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageValue.h"

registerMooseObject("MooseApp", ElementAverageValue);

InputParameters
ElementAverageValue::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Computes the volumetric average of a variable");
  return params;
}

ElementAverageValue::ElementAverageValue(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _volume(0)
{
}

void
ElementAverageValue::initialize()
{
  ElementIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
ElementAverageValue::execute()
{
  ElementIntegralVariablePostprocessor::execute();
  _volume += _current_elem_volume;
}

Real
ElementAverageValue::getValue()
{
  return _integral_value / _volume;
}

void
ElementAverageValue::finalize()
{
  gatherSum(_volume);
  gatherSum(_integral_value);
}

void
ElementAverageValue::threadJoin(const UserObject & y)
{
  ElementIntegralVariablePostprocessor::threadJoin(y);
  const ElementAverageValue & pps = static_cast<const ElementAverageValue &>(y);
  _volume += pps._volume;
}

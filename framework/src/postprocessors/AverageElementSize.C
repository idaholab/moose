//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageElementSize.h"

template <>
InputParameters
validParams<AverageElementSize>()
{
  InputParameters params = validParams<ElementAverageValue>();
  return params;
}

AverageElementSize::AverageElementSize(const InputParameters & parameters)
  : ElementAverageValue(parameters)
{
}

void
AverageElementSize::initialize()
{
  ElementAverageValue::initialize();
  _elems = 0;
}

void
AverageElementSize::execute()
{
  ElementIntegralPostprocessor::execute();
  _elems++;
}

Real
AverageElementSize::computeIntegral()
{
  return _current_elem->hmax();
}

Real
AverageElementSize::getValue()
{
  Real integral = ElementIntegralPostprocessor::getValue();

  gatherSum(_elems);

  return integral / _elems;
}

void
AverageElementSize::threadJoin(const UserObject & y)
{
  ElementAverageValue::threadJoin(y);
  const AverageElementSize & pps = static_cast<const AverageElementSize &>(y);
  _elems += pps._elems;
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageFunctorPostprocessor.h"

registerMooseObject("MooseApp", ElementAverageFunctorPostprocessor);

InputParameters
ElementAverageFunctorPostprocessor::validParams()
{
  return ElementIntegralFunctorPostprocessor::validParams();
}

ElementAverageFunctorPostprocessor::ElementAverageFunctorPostprocessor(
    const InputParameters & parameters)
  : ElementIntegralFunctorPostprocessor(parameters)
{
}

void
ElementAverageFunctorPostprocessor::initialize()
{
  ElementIntegralFunctorPostprocessor::initialize();
  _volume = 0;
}

void
ElementAverageFunctorPostprocessor::execute()
{
  ElementIntegralFunctorPostprocessor::execute();
  _volume += _current_elem_volume;
}

Real
ElementAverageFunctorPostprocessor::getValue() const
{
  return ElementIntegralFunctorPostprocessor::getValue() / _volume;
}

void
ElementAverageFunctorPostprocessor::finalize()
{
  ElementIntegralFunctorPostprocessor::finalize();
  gatherSum(_volume);
}

void
ElementAverageFunctorPostprocessor::threadJoin(const UserObject & y)
{
  ElementIntegralFunctorPostprocessor::threadJoin(y);
  const auto & pps = static_cast<const ElementAverageFunctorPostprocessor &>(y);
  _volume += pps._volume;
}

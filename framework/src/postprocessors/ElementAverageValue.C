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

#include "ElementAverageValue.h"

template<>
InputParameters validParams<ElementAverageValue>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  return params;
}

ElementAverageValue::ElementAverageValue(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),
    _volume(0)
{}

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
  Real integral = ElementIntegralVariablePostprocessor::getValue();

  gatherSum(_volume);

  return integral / _volume;
}

void
ElementAverageValue::threadJoin(const UserObject & y)
{
  ElementIntegralVariablePostprocessor::threadJoin(y);
  const ElementAverageValue & pps = static_cast<const ElementAverageValue &>(y);
  _volume += pps._volume;
}

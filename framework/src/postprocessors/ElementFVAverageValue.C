//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementFVAverageValue.h"

registerMooseObject("MooseApp", ElementFVAverageValue);

InputParameters
ElementFVAverageValue::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>("functor",
                                            "The name of the functor that this object operates on");
  params.addClassDescription(
      "Computes a volume average of the specified functor. This functor will take element spatial "
      "arguments and does not use quadrature points");
  return params;
}

ElementFVAverageValue::ElementFVAverageValue(const InputParameters & parameters)
  : ElementPostprocessor(parameters), _functor(getFunctor<Real>("functor"))
{
}

void
ElementFVAverageValue::initialize()
{
  _integral = 0;
  _volume = 0;
}

void
ElementFVAverageValue::execute()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state_arg = determineState();
  const auto & elem_info = _mesh.elemInfo(_current_elem->id());
  const auto elem_volume = elem_info.volume() * elem_info.coordFactor();
  _integral += _functor(elem_arg, state_arg) * elem_volume;
  _volume += elem_volume;
}

void
ElementFVAverageValue::finalize()
{
  gatherSum(_integral);
  gatherSum(_volume);
  _avg = _integral / _volume;
}

void
ElementFVAverageValue::threadJoin(const UserObject & y)
{
  const auto & pp = static_cast<const ElementFVAverageValue &>(y);
  _integral += pp._integral;
  _volume += pp._volume;
}

Real
ElementFVAverageValue::getValue() const
{
  return _avg;
}

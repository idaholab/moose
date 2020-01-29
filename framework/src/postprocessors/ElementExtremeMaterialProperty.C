//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeMaterialProperty.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", ElementExtremeMaterialProperty);

InputParameters
ElementExtremeMaterialProperty::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Material property for which to find extreme");
  MooseEnum type_options("max=0 min=1");
  params.addRequiredParam<MooseEnum>("value_type",
                                     type_options,
                                     "Type of extreme value to return: 'max' "
                                     "returns the maximum value and 'min' returns "
                                     "the minimum value.");

  params.addClassDescription(
      "Determines the minimum or maximum of a material property over a volume.");

  return params;
}

ElementExtremeMaterialProperty::ElementExtremeMaterialProperty(const InputParameters & parameters)
  : ElementPostprocessor(parameters),

    _mat_prop(getMaterialProperty<Real>("mat_prop")),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _value(_type == 0 ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max()),
    _qp(0)
{
}

void
ElementExtremeMaterialProperty::initialize()
{
  switch (_type)
  {
    case MAX:
      _value = -std::numeric_limits<Real>::max(); // start w/ the min
      break;

    case MIN:
      _value = std::numeric_limits<Real>::max(); // start w/ the max
      break;
  }
}

void
ElementExtremeMaterialProperty::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpValue();
}

void
ElementExtremeMaterialProperty::computeQpValue()
{
  switch (_type)
  {
    case MAX:
      _value = std::max(_value, _mat_prop[_qp]);
      break;

    case MIN:
      _value = std::min(_value, _mat_prop[_qp]);
      break;
  }
}

Real
ElementExtremeMaterialProperty::getValue()
{
  switch (_type)
  {
    case MAX:
      gatherMax(_value);
      break;
    case MIN:
      gatherMin(_value);
      break;
  }

  return _value;
}

void
ElementExtremeMaterialProperty::threadJoin(const UserObject & y)
{
  const ElementExtremeMaterialProperty & pps =
      static_cast<const ElementExtremeMaterialProperty &>(y);

  switch (_type)
  {
    case MAX:
      _value = std::max(_value, pps._value);
      break;
    case MIN:
      _value = std::min(_value, pps._value);
      break;
  }
}

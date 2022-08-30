//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeMaterialProperty.h"

#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", ElementExtremeMaterialProperty);
registerMooseObject("MooseApp", ADElementExtremeMaterialProperty);

template <bool is_ad>
InputParameters
ElementExtremeMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Material property for which to find the extreme");
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

template <bool is_ad>
ElementExtremeMaterialPropertyTempl<is_ad>::ElementExtremeMaterialPropertyTempl(
    const InputParameters & parameters)
  : ElementPostprocessor(parameters),

    _mat_prop(getGenericMaterialProperty<Real, is_ad>("mat_prop")),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _value(_type == 0 ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max()),
    _qp(0)
{
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyTempl<is_ad>::initialize()
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

template <bool is_ad>
void
ElementExtremeMaterialPropertyTempl<is_ad>::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpValue();
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyTempl<is_ad>::computeQpValue()
{
  switch (_type)
  {
    case MAX:
      _value = std::max(_value, MetaPhysicL::raw_value(_mat_prop[_qp]));
      break;

    case MIN:
      _value = std::min(_value, MetaPhysicL::raw_value(_mat_prop[_qp]));
      break;
  }
}

template <bool is_ad>
Real
ElementExtremeMaterialPropertyTempl<is_ad>::getValue()
{
  return _value;
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyTempl<is_ad>::finalize()
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
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyTempl<is_ad>::threadJoin(const UserObject & y)
{
  const ElementExtremeMaterialPropertyTempl<is_ad> & pps =
      static_cast<const ElementExtremeMaterialPropertyTempl<is_ad> &>(y);

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

template class ElementExtremeMaterialPropertyTempl<false>;
template class ElementExtremeMaterialPropertyTempl<true>;

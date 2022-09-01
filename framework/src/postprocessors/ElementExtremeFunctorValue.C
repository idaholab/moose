//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeFunctorValue.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", ElementExtremeFunctorValue);

registerMooseObject("MooseApp", ADElementExtremeFunctorValue);

template <bool is_ad>
InputParameters
ElementExtremeFunctorValueTempl<is_ad>::validParams()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "max");

  // Define the parameters
  InputParameters params = ElementPostprocessor::validParams();

  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return. 'max' "
                             "returns the maximum value. 'min' returns "
                             "the minimum value.");

  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor for which to find the extrema");
  params.addParam<MooseFunctorName>(
      "proxy_functor",
      "The name of the functor to use to identify the location at which "
      "the functor value should be taken; if not provided, this defaults "
      "to the 'functor' parameter.");

  params.addClassDescription(
      "Finds either the min or max elemental value of a variable over the domain.");

  return params;
}

template <bool is_ad>
ElementExtremeFunctorValueTempl<is_ad>::ElementExtremeFunctorValueTempl(
    const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _proxy_functor(isParamValid("proxy_functor") ? getFunctor<GenericReal<is_ad>>("proxy_functor")
                                                 : getFunctor<GenericReal<is_ad>>("functor"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

template <bool is_ad>
void
ElementExtremeFunctorValueTempl<is_ad>::initialize()
{
  switch (_type)
  {
    case MAX:
      _proxy_value =
          std::make_pair(-std::numeric_limits<Real>::max(), -std::numeric_limits<Real>::max());
      break;

    case MIN:
      _proxy_value =
          std::make_pair(std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max());
      break;
  }
}

template <bool is_ad>
void
ElementExtremeFunctorValueTempl<is_ad>::computeValue()
{
  // Most element evaluations do not use skewness correction,
  // but this could become a parameter in the future
  Moose::ElemArg elem = makeElemArg(_current_elem);
  const auto pv = std::make_pair(MetaPhysicL::raw_value(_proxy_functor(elem)),
                                 MetaPhysicL::raw_value(_functor(elem)));
  switch (_type)
  {
    case MAX:
      if (pv > _proxy_value)
        _proxy_value = pv;
      break;

    case MIN:
      if (pv < _proxy_value)
        _proxy_value = pv;
      break;
  }
}

template <bool is_ad>
Real
ElementExtremeFunctorValueTempl<is_ad>::getValue()
{
  return _proxy_value.second;
}

template <bool is_ad>
void
ElementExtremeFunctorValueTempl<is_ad>::finalize()
{
  switch (_type)
  {
    case MAX:
      gatherProxyValueMax(_proxy_value.first, _proxy_value.second);
      break;
    case MIN:
      gatherProxyValueMin(_proxy_value.first, _proxy_value.second);
      break;
  }
}

template <bool is_ad>
void
ElementExtremeFunctorValueTempl<is_ad>::threadJoin(const UserObject & y)
{
  const ElementExtremeFunctorValueTempl<is_ad> & pps =
      static_cast<const ElementExtremeFunctorValueTempl<is_ad> &>(y);

  switch (_type)
  {
    case MAX:
      if (pps._proxy_value > _proxy_value)
        _proxy_value = pps._proxy_value;
      break;
    case MIN:
      if (pps._proxy_value < _proxy_value)
        _proxy_value = pps._proxy_value;
      break;
  }
}

template class ElementExtremeFunctorValueTempl<false>;
template class ElementExtremeFunctorValueTempl<true>;

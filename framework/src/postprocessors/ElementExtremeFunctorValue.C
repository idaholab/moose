//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeFunctorValue.h"

registerMooseObject("MooseApp", ElementExtremeFunctorValue);
registerMooseObject("MooseApp", ADElementExtremeFunctorValue);

template <bool is_ad>
InputParameters
ElementExtremeFunctorValueTempl<is_ad>::validParams()
{
  InputParameters params = ExtremeValueBase<ElementPostprocessor>::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor for which to find the extrema");
  params.addParam<MooseFunctorName>(
      "proxy_functor",
      "The name of the functor to use to identify the location at which "
      "the functor value should be taken; if not provided, this defaults "
      "to the 'functor' parameter.");
  params.addClassDescription(
      "Finds either the min or max elemental value of a functor over the domain.");
  return params;
}

template <bool is_ad>
ElementExtremeFunctorValueTempl<is_ad>::ElementExtremeFunctorValueTempl(
    const InputParameters & parameters)
  : ExtremeValueBase<ElementPostprocessor>(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _proxy_functor(isParamValid("proxy_functor") ? getFunctor<GenericReal<is_ad>>("proxy_functor")
                                                 : getFunctor<GenericReal<is_ad>>("functor"))
{
  _use_proxy = isParamValid("proxy_functor");
}

template <bool is_ad>
std::pair<Real, Real>
ElementExtremeFunctorValueTempl<is_ad>::getProxyValuePair()
{
  // Most element evaluations do not use skewness correction,
  // but this could become a parameter in the future
  Moose::ElemArg elem = makeElemArg(_current_elem);
  return std::make_pair(MetaPhysicL::raw_value(_proxy_functor(elem, determineState())),
                        MetaPhysicL::raw_value(_functor(elem, determineState())));
}

template class ElementExtremeFunctorValueTempl<false>;
template class ElementExtremeFunctorValueTempl<true>;

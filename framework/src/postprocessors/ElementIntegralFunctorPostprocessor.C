//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralFunctorPostprocessor.h"

registerMooseObject("MooseApp", ElementIntegralFunctorPostprocessor);
registerMooseObject("MooseApp", ADElementIntegralFunctorPostprocessor);

template <bool is_ad>
InputParameters
ElementIntegralFunctorPostprocessorTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>("functor",
                                            "The name of the functor that this object operates on");
  params.addParam<MooseFunctorName>(
      "prefactor", 1, "The name of a pre-factor inside the integrand");
  params.addClassDescription("Computes a volume integral of the specified functor");
  return params;
}

template <bool is_ad>
ElementIntegralFunctorPostprocessorTempl<is_ad>::ElementIntegralFunctorPostprocessorTempl(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _prefactor(getFunctor<GenericReal<is_ad>>("prefactor"))
{
}

template <bool is_ad>
Real
ElementIntegralFunctorPostprocessorTempl<is_ad>::computeQpIntegral()
{
  Moose::ElemQpArg elem_qp = {_current_elem, _qp, _qrule};
  return MetaPhysicL::raw_value(_prefactor(elem_qp, determineState()) *
                                _functor(elem_qp, determineState()));
}

template class ElementIntegralFunctorPostprocessorTempl<false>;
template class ElementIntegralFunctorPostprocessorTempl<true>;

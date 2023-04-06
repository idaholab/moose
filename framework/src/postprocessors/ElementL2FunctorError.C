//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementL2FunctorError.h"
#include "MooseFunctor.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", ElementL2FunctorError);
registerMooseObject("MooseApp", ADElementL2FunctorError);

template <bool is_ad>
InputParameters
ElementL2FunctorErrorTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>("approximate",
                                            "The approximate functor. This functor has to be an "
                                            "ADFunctor, like a variable or an ADFunction");
  params.addRequiredParam<MooseFunctorName>("exact", "The analytic solution to compare against");
  params.addClassDescription(
      "Computes L2 error between an 'approximate' functor and an 'exact' functor");
  return params;
}

template <bool is_ad>
ElementL2FunctorErrorTempl<is_ad>::ElementL2FunctorErrorTempl(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _approx(getFunctor<ADReal>("approximate")),
    _exact(getFunctor<GenericReal<is_ad>>("exact"))
{
}

template <bool is_ad>
Real
ElementL2FunctorErrorTempl<is_ad>::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

template <bool is_ad>
Real
ElementL2FunctorErrorTempl<is_ad>::computeQpIntegral()
{
  Moose::ElemQpArg elem_qp = {_current_elem, _qp, _qrule};
  Real diff = MetaPhysicL::raw_value(_approx(elem_qp, determineState())) -
              MetaPhysicL::raw_value(_exact(elem_qp, determineState()));
  return diff * diff;
}

template class ElementL2FunctorErrorTempl<false>;
template class ElementL2FunctorErrorTempl<true>;

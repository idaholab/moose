//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  MooseEnum evaluation_type(getFunctorEvaluationTypeOptions(), "QUADRATURE_POINT");
  params.addParam<MooseEnum>(
      "evaluation_type", evaluation_type, "How the functor should be evaluated.");
  return params;
}

template <bool is_ad>
ElementIntegralFunctorPostprocessorTempl<is_ad>::ElementIntegralFunctorPostprocessorTempl(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _prefactor(getFunctor<GenericReal<is_ad>>("prefactor")),
    _evaluation_type(
        getParam<MooseEnum>("evaluation_type").template getEnum<FunctorEvaluationType>())
{
}

template <bool is_ad>
Real
ElementIntegralFunctorPostprocessorTempl<is_ad>::computeIntegral()
{
  switch (_evaluation_type)
  {
    case FunctorEvaluationType::QUADRATURE_POINT:
      return ElementIntegralPostprocessor::computeIntegral();
    case FunctorEvaluationType::CELL_AVERAGE:
      return _current_elem_volume * cellAverage();
    default:
      mooseError("Unhandled enumerator type");
  }
}

template <bool is_ad>
Real
ElementIntegralFunctorPostprocessorTempl<is_ad>::computeQpIntegral()
{
  Moose::ElemQpArg elem_qp = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return MetaPhysicL::raw_value(_prefactor(elem_qp, determineState()) *
                                _functor(elem_qp, determineState()));
}

template <bool is_ad>
Real
ElementIntegralFunctorPostprocessorTempl<is_ad>::cellAverage()
{
  const Moose::ElemArg elem_arg = makeElemArg(_current_elem);
  return MetaPhysicL::raw_value(_prefactor(elem_arg, determineState()) *
                                _functor(elem_arg, determineState()));
}

template class ElementIntegralFunctorPostprocessorTempl<false>;
template class ElementIntegralFunctorPostprocessorTempl<true>;

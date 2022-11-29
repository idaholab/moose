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

InputParameters
ElementL2FunctorError::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>("approximate", "The approximate functor");
  params.addRequiredParam<MooseFunctorName>("exact", "The analytic solution to compare against");
  params.addClassDescription(
      "Computes L2 error between an 'approximate' functor and an 'exact' functor");
  return params;
}

ElementL2FunctorError::ElementL2FunctorError(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _approx(getFunctor<ADReal>("approximate")),
    _exact(getFunctor<ADReal>("exact"))
{
}

Real
ElementL2FunctorError::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementL2FunctorError::computeQpIntegral()
{
  Moose::ElemQpArg elem_qp = {_current_elem, _qp, _qrule};
  Real diff = MetaPhysicL::raw_value(_approx(elem_qp)) - MetaPhysicL::raw_value(_exact(elem_qp));
  return diff * diff;
}

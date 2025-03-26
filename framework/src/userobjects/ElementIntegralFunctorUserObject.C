//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralFunctorUserObject.h"

InputParameters
ElementIntegralFunctorUserObject::validParams()
{
  InputParameters params = ElementIntegralUserObject::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to be integrated");
  return params;
}

ElementIntegralFunctorUserObject::ElementIntegralFunctorUserObject(
    const InputParameters & parameters)
  : ElementIntegralUserObject(parameters),
    NonADFunctorInterface(this),
    _functor(getFunctor<Real>("functor"))
{
}

Real
ElementIntegralFunctorUserObject::computeQpIntegral()
{
  const Moose::ElemQpArg elem_qp = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return _functor(elem_qp, determineState());
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralFunctorUserObject.h"

InputParameters
SideIntegralFunctorUserObject::validParams()
{
  InputParameters params = SideIntegralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to be integrated");
  return params;
}

SideIntegralFunctorUserObject::SideIntegralFunctorUserObject(const InputParameters & parameters)
  : SideIntegralUserObject(parameters),
    NonADFunctorInterface(this),
    _functor(getFunctor<Real>("functor"))
{
}

Real
SideIntegralFunctorUserObject::computeQpIntegral()
{
  Moose::ElemSideQpArg elem_side_qp = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  return _functor(elem_side_qp, determineState());
}

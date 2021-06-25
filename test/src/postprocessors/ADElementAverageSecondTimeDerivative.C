//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADElementAverageSecondTimeDerivative.h"

registerMooseObject("MooseTestApp", ADElementAverageSecondTimeDerivative);

InputParameters
ADElementAverageSecondTimeDerivative::validParams()
{
  InputParameters params = ElementAverageValue::validParams();
  params.addClassDescription("Computes the element averaged second derivative of variable");
  return params;
}

ADElementAverageSecondTimeDerivative::ADElementAverageSecondTimeDerivative(
    const InputParameters & parameters)
  : ElementAverageValue(parameters),
    _u_dotdot(_is_transient ? adCoupledDotDot("variable") : _ad_zero)
{
}

Real
ADElementAverageSecondTimeDerivative::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_u_dotdot[_qp]);
}

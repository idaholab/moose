//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableInnerProduct.h"

registerMooseObject("MooseApp", VariableInnerProduct);

InputParameters
VariableInnerProduct::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredCoupledVar(
      "second_variable",
      "The name of the second variable in the inner product (variable, second_variable)");
  return params;
}

VariableInnerProduct::VariableInnerProduct(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _v(coupledValue("second_variable"))
{
}

Real
VariableInnerProduct::computeQpIntegral()
{
  return _u[_qp] * _v[_qp];
}

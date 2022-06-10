//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementW1pError.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementW1pError);

InputParameters
ElementW1pError::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1", "The exponent used in the norm.");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  params.addClassDescription("Computes the W1p norm of the difference between a variable and an "
                             "analytic solution, as a function");
  return params;
}

ElementW1pError::ElementW1pError(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _p(getParam<Real>("p")),
    _func(getFunction("function"))
{
}

Real
ElementW1pError::getValue()
{
  return std::pow(ElementIntegralPostprocessor::getValue(), 1. / _p);
}

Real
ElementW1pError::computeQpIntegral()
{
  RealGradient graddiff = _grad_u[_qp] - _func.gradient(_t, _q_point[_qp]);
  Real funcdiff = _u[_qp] - _func.value(_t, _q_point[_qp]);

  // Raise the absolute function value difference to the pth power
  Real val = std::pow(std::abs(funcdiff), _p);

  // Add all of the absolute gradient component differences to the pth power
  for (const auto i : make_range(Moose::dim))
    val += std::pow(std::abs(graddiff(i)), _p);

  return val;
}

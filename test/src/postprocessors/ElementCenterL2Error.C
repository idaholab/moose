//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementCenterL2Error.h"
#include "Function.h"

registerMooseObject("MooseTestApp", ElementCenterL2Error);

InputParameters
ElementCenterL2Error::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  params.addClassDescription("Computes L2 error between a field variable and an analytical "
                             "function at the centroid of the element");
  return params;
}

ElementCenterL2Error::ElementCenterL2Error(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _func(getFunction("function"))
{
}

Real
ElementCenterL2Error::getValue() const
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementCenterL2Error::computeQpIntegral()
{
  Real diff = _u[_qp] - _func.value(_t, _current_elem->vertex_average());
  return diff * diff;
}

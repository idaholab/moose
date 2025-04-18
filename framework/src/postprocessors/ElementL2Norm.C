//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementL2Norm.h"

registerMooseObject("MooseApp", ElementL2Norm);

InputParameters
ElementL2Norm::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  return params;
}

ElementL2Norm::ElementL2Norm(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters)
{
}

Real
ElementL2Norm::getValue() const
{
  return std::sqrt(ElementIntegralVariablePostprocessor::getValue());
}

Real
ElementL2Norm::computeQpIntegral()
{
  Real val = _u[_qp];
  return val * val;
}

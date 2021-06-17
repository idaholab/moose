//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementArrayL2Norm.h"

registerMooseObject("MooseApp", ElementArrayL2Norm);

InputParameters
ElementArrayL2Norm::validParams()
{
  InputParameters params = ElementIntegralArrayVariablePostprocessor::validParams();
  params.addClassDescription("Evaluates L2-norm of a component of an array variable");
  return params;
}

ElementArrayL2Norm::ElementArrayL2Norm(const InputParameters & parameters)
  : ElementIntegralArrayVariablePostprocessor(parameters)
{
}

Real
ElementArrayL2Norm::getValue()
{
  return std::sqrt(ElementIntegralArrayVariablePostprocessor::getValue());
}

Real
ElementArrayL2Norm::computeQpIntegral()
{
  Real val = _u[_qp](_component);
  return val * val;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSidesL2Norm.h"

registerMooseObject("MooseApp", ElementSidesL2Norm);

InputParameters
ElementSidesL2Norm::validParams()
{
  InputParameters params = InternalSideIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Computes the L2 norm of a variable over element sides.");
  return params;
}

ElementSidesL2Norm::ElementSidesL2Norm(const InputParameters & parameters)
  : InternalSideIntegralVariablePostprocessor(parameters)
{
}

Real
ElementSidesL2Norm::getValue()
{
  return std::sqrt(InternalSideIntegralVariablePostprocessor::getValue());
}

Real
ElementSidesL2Norm::computeQpIntegral()
{
  auto & u = _u[_qp];
  return u * u;
}

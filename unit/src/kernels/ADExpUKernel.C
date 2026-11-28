//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADExpUKernel.h"

registerMooseObject("MooseUnitApp", ADExpUKernel);

InputParameters
ADExpUKernel::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Field kernel source density: exp(u - 1).");
  return params;
}

ADExpUKernel::ADExpUKernel(const InputParameters & parameters) : ADKernelValue(parameters) {}

ADReal
ADExpUKernel::precomputeQpResidual()
{
  using std::exp;
  return exp(_u[_qp] - 1.0);
}

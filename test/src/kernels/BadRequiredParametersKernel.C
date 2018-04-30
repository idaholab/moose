//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BadRequiredParametersKernel.h"

registerMooseObject("MooseTestApp", BadRequiredParametersKernel);

template <>
InputParameters
validParams<BadRequiredParametersKernel>()
{
  InputParameters params = validParams<Kernel>();
  // this should raise a run-time error
  params.makeParamRequired<Real>("name");
  return params;
}

BadRequiredParametersKernel::BadRequiredParametersKernel(const InputParameters & parameters)
  : Kernel(parameters)
{
}

Real
BadRequiredParametersKernel::computeQpResidual()
{
  return 0.0;
}

Real
BadRequiredParametersKernel::computeQpJacobian()
{
  return 0.0;
}

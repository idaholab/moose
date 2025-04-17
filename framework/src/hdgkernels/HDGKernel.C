//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGKernel.h"

InputParameters
HDGKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.registerBase("HDGKernel");
  return params;
}

HDGKernel::HDGKernel(const InputParameters & parameters) : Kernel(parameters) {}

void
HDGKernel::computeResidualAndJacobianOnSide()
{
  mooseError("not implemented");
}

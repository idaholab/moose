//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVQpElementalKernel.h"

InputParameters
FVQpElementalKernel::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  return params;
}

FVQpElementalKernel::FVQpElementalKernel(const InputParameters & parameters)
  : FVElementalKernel(parameters), _u(_var.adSln())
{
}

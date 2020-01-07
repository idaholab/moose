//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DoNotCopyParametersKernel.h"

registerMooseObject("MooseTestApp", DoNotCopyParametersKernel);

InputParameters
DoNotCopyParametersKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

// This is the wrong constructor, don't to this!
DoNotCopyParametersKernel::DoNotCopyParametersKernel(InputParameters parameters)
  : Kernel(parameters)
{
}

Real
DoNotCopyParametersKernel::computeQpResidual()
{
  getParam<std::string>("name"); // This will cause a segmentation fault
  return 0.0;
}

Real
DoNotCopyParametersKernel::computeQpJacobian()
{
  return 0.0;
}

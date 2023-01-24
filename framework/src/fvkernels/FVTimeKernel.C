//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVTimeKernel.h"

#include "SystemBase.h"

registerADMooseObject("MooseApp", FVTimeKernel);

InputParameters
FVTimeKernel::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Residual contribution from time derivative of a variable for the finite volume method.");
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

FVTimeKernel::FVTimeKernel(const InputParameters & parameters) : FVElementalKernel(parameters) {}

ADReal
FVTimeKernel::computeQpResidual()
{
  return _var.dot(makeElemArg(_current_elem));
}

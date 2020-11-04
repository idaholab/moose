//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayReaction.h"

registerADMooseObject("MooseApp", FVArrayReaction);

InputParameters
FVArrayReaction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Simple consuming reaction term");
  return params;
}

FVArrayReaction::FVArrayReaction(const InputParameters & parameters)
  : FVArrayElementalKernel(parameters)
{
}

ADRealEigenVector
FVArrayReaction::computeQpResidual()
{
  return _u[_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVReaction.h"

registerADMooseObject("MooseApp", FVReaction);

InputParameters
FVReaction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Simple consuming reaction term");
  return params;
}

FVReaction::FVReaction(const InputParameters & parameters) : FVElementalKernel(parameters) {}

ADReal
FVReaction::computeQpResidual()
{
  return _u[_qp];
}

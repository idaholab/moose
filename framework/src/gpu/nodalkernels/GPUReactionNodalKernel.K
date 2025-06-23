//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUReactionNodalKernel.h"

registerMooseObject("MooseApp", GPUReactionNodalKernel);

InputParameters
GPUReactionNodalKernel::validParams()
{
  InputParameters params = GPUNodalKernel::validParams();
  params.addClassDescription("Implements a simple consuming reaction term at nodes");
  params.addParam<Real>("coeff", 1., "A coefficient for multiplying the reaction term");
  return params;
}

GPUReactionNodalKernel::GPUReactionNodalKernel(const InputParameters & parameters)
  : GPUNodalKernel(parameters), _coeff(getParam<Real>("coeff"))
{
}

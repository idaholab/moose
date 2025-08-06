//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosReactionNodalKernel.h"

registerMooseObject("MooseApp", KokkosReactionNodalKernel);

InputParameters
KokkosReactionNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Implements a simple consuming reaction term at nodes");
  params.addParam<Real>("coeff", 1., "A coefficient for multiplying the reaction term");
  return params;
}

KokkosReactionNodalKernel::KokkosReactionNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _coeff(getParam<Real>("coeff"))
{
}

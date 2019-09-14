//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EmptyKernel.h"

registerMooseObject("PeridynamicsApp", EmptyKernel);

template <>
InputParameters
validParams<EmptyKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Empty kernel class for element whose residual does not need to be calculated");

  return params;
}

EmptyKernel::EmptyKernel(const InputParameters & parameters) : Kernel(parameters) {}

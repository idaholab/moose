//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEMFluidProperties.h"
#include "FluidProperties.h"

template <>
InputParameters
validParams<HEMFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();
  return params;
}

HEMFluidProperties::HEMFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

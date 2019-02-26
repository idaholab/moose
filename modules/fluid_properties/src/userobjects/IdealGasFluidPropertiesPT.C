//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesPT.h"

registerMooseObject("FluidPropertiesApp", IdealGasFluidPropertiesPT);

template <>
InputParameters
validParams<IdealGasFluidPropertiesPT>()
{
  InputParameters params = validParams<IdealGasFluidProperties>();
  return params;
}

IdealGasFluidPropertiesPT::IdealGasFluidPropertiesPT(const InputParameters & parameters)
  : IdealGasFluidProperties(parameters)
{
  mooseDeprecated(
      name(),
      ": IdealGasFluidPropertiesPT is deprecated. Just use IdealGasFluidProperties instead");
}

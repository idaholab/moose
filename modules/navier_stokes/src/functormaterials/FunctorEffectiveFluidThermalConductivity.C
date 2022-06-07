//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorEffectiveFluidThermalConductivity.h"

InputParameters
FunctorEffectiveFluidThermalConductivity::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Base class for computing effective fluid thermal conductivity");
  return params;
}

FunctorEffectiveFluidThermalConductivity::FunctorEffectiveFluidThermalConductivity(
    const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
}

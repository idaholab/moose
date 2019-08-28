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
  params.addCustomTypeParam<std::string>(
      "fp_type", "hem-fp", "FPType", "Type of the fluid property object");
  return params;
}

HEMFluidProperties::HEMFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

Real
HEMFluidProperties::molarMass() const
{
  mooseError(name(), ": molarMass is not implemented");
}
Real
HEMFluidProperties::criticalPressure() const
{
  mooseError(name(), ": criticalPressure() is not implemented");
}

Real
HEMFluidProperties::criticalTemperature() const
{
  mooseError(name(), ": criticalTemperature() is not implemented");
}

Real
HEMFluidProperties::criticalDensity() const
{
  mooseError(name(), ": criticalDensity() is not implemented");
}

Real
HEMFluidProperties::criticalInternalEnergy() const
{
  mooseError(name(), ": criticalInternalEnergy() is not implemented");
}

Real
HEMFluidProperties::triplePointPressure() const
{
  mooseError(name(), ": triplePointPressure() is not implemented");
}

Real
HEMFluidProperties::triplePointTemperature() const
{
  mooseError(name(), ": triplePointTemperature() is not implemented");
}

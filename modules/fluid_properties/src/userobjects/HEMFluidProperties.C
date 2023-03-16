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

InputParameters
HEMFluidProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();
  params.set<std::string>("fp_type") = "hem-fp";

  return params;
}

HEMFluidProperties::HEMFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

Real
HEMFluidProperties::molarMass() const
{
  mooseError("molarMass is not implemented");
}
Real
HEMFluidProperties::criticalPressure() const
{
  mooseError("criticalPressure() is not implemented");
}

Real
HEMFluidProperties::criticalTemperature() const
{
  mooseError("criticalTemperature() is not implemented");
}

Real
HEMFluidProperties::criticalDensity() const
{
  mooseError("criticalDensity() is not implemented");
}

Real
HEMFluidProperties::criticalInternalEnergy() const
{
  mooseError("criticalInternalEnergy() is not implemented");
}

Real
HEMFluidProperties::triplePointPressure() const
{
  mooseError("triplePointPressure() is not implemented");
}

Real
HEMFluidProperties::triplePointTemperature() const
{
  mooseError("triplePointTemperature() is not implemented");
}

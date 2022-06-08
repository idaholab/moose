//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesMaterial.h"

InputParameters
SolidPropertiesMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Base class for defining solid property materials");
  return params;
}

SolidPropertiesMaterial::SolidPropertiesMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters)
{
}

Real
SolidPropertiesMaterial::molarMass() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeIsobaricSpecificHeat()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeIsobaricSpecificHeatDerivatives()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalConductivity()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalConductivityDerivatives()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeDensity()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeDensityDerivatives()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalExpansionCoefficient()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeSurfaceEmissivity()
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

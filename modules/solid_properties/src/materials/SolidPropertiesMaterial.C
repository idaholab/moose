//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesMaterial.h"

const std::string SolidPropertiesMaterial::_name = "";

template <>
InputParameters
validParams<SolidPropertiesMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Base class for defining solid property materials");
  return params;
}

SolidPropertiesMaterial::SolidPropertiesMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters)
{
}

const std::string &
SolidPropertiesMaterial::solidName() const
{
  return _name;
}

Real
SolidPropertiesMaterial::molarMass() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeIsobaricSpecificHeat()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeIsobaricSpecificHeatDerivatives()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalConductivity()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalConductivityDerivatives()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeDensity()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeDensityDerivatives()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeThermalExpansionCoefficient()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SolidPropertiesMaterial::computeSurfaceEmissivity()
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

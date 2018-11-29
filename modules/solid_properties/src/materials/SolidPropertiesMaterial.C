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
  params.registerBase("SolidPropertiesMaterial");
  params.addClassDescription("Base class for defining solid property materials");
  return params;
}

SolidPropertiesMaterial::SolidPropertiesMaterial(const InputParameters & parameters)
  : Material(parameters)
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

Real
SolidPropertiesMaterial::cp() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SolidPropertiesMaterial::k() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SolidPropertiesMaterial::rho() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SolidPropertiesMaterial::beta() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SolidPropertiesMaterial::surface_emissivity() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

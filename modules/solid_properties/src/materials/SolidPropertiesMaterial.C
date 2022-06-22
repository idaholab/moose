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

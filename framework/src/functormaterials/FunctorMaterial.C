//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorMaterial.h"

InputParameters
FunctorMaterial::validParams()
{
  auto params = Material::validParams();
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

FunctorMaterial::FunctorMaterial(const InputParameters & parameters) : Material(parameters) {}

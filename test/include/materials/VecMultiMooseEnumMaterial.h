//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "MaterialProperty.h"

/**
 * Simple material to test vector parameter range checking.
 */
class VecMultiMooseEnumMaterial : public Material
{
public:
  static InputParameters validParams();

  VecMultiMooseEnumMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties();
};

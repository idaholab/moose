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
#include "DerivativeMaterialInterface.h"

/**
 * Base class for defining solid property materials.
 */
class SolidPropertiesMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  SolidPropertiesMaterial(const InputParameters & parameters);
};

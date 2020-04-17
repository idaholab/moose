//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMatDiffusionBase.h"

/**
 * Anisotropic diffusion kernel that takes a diffusion coefficient of type
 * RealTensorValue. All logic is implemnted in the MatDiffusionBase class
 * template.
 */
class ADMatAnisoDiffusion : public ADMatDiffusionBase<RealTensorValue>
{
public:
  static InputParameters validParams();

  ADMatAnisoDiffusion(const InputParameters & parameters);
};

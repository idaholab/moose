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
 * Isotropic diffusion kernel that takes a diffusion coefficient of type
 * Real. All logic is implemnted in the ADMatDiffusionBase class
 * template.
 */
class ADMatDiffusion : public ADMatDiffusionBase<Real>
{
public:
  static InputParameters validParams();

  ADMatDiffusion(const InputParameters & parameters);
};

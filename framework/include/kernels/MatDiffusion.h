//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MatDiffusionBase.h"

/**
 * Isotropic diffusion kernel that takes a diffusion coefficient of type
 * Real. All logic is implemnted in the MatDiffusionBase class
 * template.
 */
template <bool is_ad>
class MatDiffusionTempl : public MatDiffusionBaseTempl<Real, is_ad>
{
public:
  static InputParameters validParams();

  MatDiffusionTempl(const InputParameters & parameters);
};

typedef MatDiffusionTempl<false> MatDiffusion;
typedef MatDiffusionTempl<true> ADMatDiffusion;

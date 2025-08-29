//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalMatDiffusionBase.h"

/**
 * Isotropic diffusion kernel that takes a diffusion coefficient of type
 * Real. Extends MatDiffusion to allow improved numerical performance in polycrystalline
 * microstructures by adding Jacobian contributions for cases where diffusivity depends
 * on gradients of order parameters.
 */
class PolycrystalMatDiffusion : public PolycrystalMatDiffusionBase<Real>
{
public:
  static InputParameters validParams();

  PolycrystalMatDiffusion(const InputParameters & parameters);
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDiffusion.h"

/**
 * A flux kernel for diffusion of energy in porous media across cell faces using an effective
 * diffusion coefficient
 */
class PINSFVEnergyEffectiveDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVEnergyEffectiveDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the effective thermal conductivity
  const ADMaterialProperty<Real> & _kappa_elem;
  /// the neighbor effective element thermal conductivity
  const ADMaterialProperty<Real> & _kappa_neighbor;
};

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
 * A flux kernel for diffusing energy in porous media across cell faces, using a scalar
 * isotropic diffusion coefficient, using regular material properties
 */
class PCNSFVEnergyDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PCNSFVEnergyDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the thermal conductivity
  const ADMaterialProperty<Real> & _k_elem;
  /// the neighbor element thermal conductivity
  const ADMaterialProperty<Real> & _k_neighbor;
  /// the porosity
  const VariableValue & _eps;
  /// the neighbor element porosity
  const VariableValue & _eps_neighbor;
  /// whether the diffusivity should be multiplied by porosity
  const bool _porosity_factored_in;
};

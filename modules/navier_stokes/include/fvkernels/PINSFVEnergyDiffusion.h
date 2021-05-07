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
 * A flux kernel for diffusing energy in porous media across cell faces, using a regular
 * diffusion coefficient, which is multiplied by porosity
 */
class PINSFVEnergyDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVEnergyDiffusion(const InputParameters & params);

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
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

/**
 * A flux kernel for diffusion of momentum in porous media across cell faces
 */
class PINSFVMomentumDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVMomentumDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the current element viscosity
  const ADMaterialProperty<Real> & _mu_elem;
  /// the neighbor element viscosity
  const ADMaterialProperty<Real> & _mu_neighbor;

  /// the porosity
  const VariableValue & _eps;
  /// the neighbor element porosity
  const VariableValue & _eps_neighbor;

  // Parameters for the gradient diffusion term
  /// Which momentum component this kernel applies to
  const int _index;

  /// Velocity as material properties
  const ADMaterialProperty<RealVectorValue> * _vel_elem;
  const ADMaterialProperty<RealVectorValue> * _vel_neighbor;

  /// the porosity as a variable to be able to compute a face gradient
  const MooseVariableFVReal * const _eps_var;

  /// Whether to add the porosity gradient term, only for continuous porosity
  const bool _smooth_porosity;
};

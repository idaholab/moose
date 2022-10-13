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
#include "MathFVUtils.h"

/**
 * A flux kernel for diffusing energy in porous media across cell faces, using a scalar
 * isotropic diffusion coefficient, using functor material properties
 */
class PINSFVEnergyDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVEnergyDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the thermal conductivity
  const Moose::Functor<ADReal> & _k;
  /// the porosity
  const Moose::Functor<ADReal> & _eps;
  /// whether the diffusivity should be multiplied by porosity
  const bool _porosity_factored_in;
  /// which interpolation method for the diffusivity on faces
  const Moose::FV::InterpMethod _k_interp_method;
};

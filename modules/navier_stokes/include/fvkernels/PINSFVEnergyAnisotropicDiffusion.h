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
 * A flux kernel for diffusion of energy in porous media across cell faces using a
 * vector diffusion coefficient, to model anisotropy, using functor material properties
 */
class PINSFVEnergyAnisotropicDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVEnergyAnisotropicDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the thermal conductivity
  const Moose::Functor<ADRealVectorValue> & _k;
  /// the porosity
  const Moose::Functor<ADReal> & _eps;
  /// whether the diffusivity should be multiplied by porosity
  const bool _porosity_factored_in;
  /// The face interpolation method for the conductivity
  const Moose::FV::InterpMethod _k_interp_method;
};

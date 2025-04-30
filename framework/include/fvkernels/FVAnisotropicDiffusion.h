//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "FVDiffusionInterpolationInterface.h"

/**
 * FVAnisotropicDiffusion implements a standard diffusion term
 * but with a diagonal tensor diffusion coefficient (provided as a vector)
 */
class FVAnisotropicDiffusion : public FVFluxKernel, public FVDiffusionInterpolationInterface
{
public:
  static InputParameters validParams();
  FVAnisotropicDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Functor returning the diagonal coefficients of a diffusion tensor
  const Moose::Functor<ADRealVectorValue> & _coeff;

  /// Decides if a geometric arithmetic or harmonic average is used for the
  /// face interpolation of the diffusion coefficient.
  const Moose::FV::InterpMethod _coeff_interp_method;
};

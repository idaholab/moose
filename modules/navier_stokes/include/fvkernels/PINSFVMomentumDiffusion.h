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
#include "CentralDifferenceLimiter.h"

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
  const Moose::Functor<ADReal> & _mu;

  /// the porosity
  const Moose::Functor<ADReal> & _eps;

  // Parameters for the gradient diffusion term
  /// Which momentum component this kernel applies to
  const int _index;

  /// Velocity as functors
  const Moose::Functor<ADRealVectorValue> * const _vel;

  /// the porosity as a variable to be able to compute a face gradient
  const MooseVariableFVReal * const _eps_var;

  /// Whether to add the porosity gradient term, only for continuous porosity
  const bool _smooth_porosity;

  /// The object used to perform average/central-difference interpolations
  Moose::FV::CentralDifferenceLimiter<ADReal> _cd_limiter;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumDiffusion.h"

/**
 * A flux kernel for diffusion of momentum in porous media across cell faces
 */
class PINSFVMomentumDiffusionComplete : public PINSFVMomentumDiffusion
{
public:
  static InputParameters validParams();
  PINSFVMomentumDiffusionComplete(const InputParameters & params);

protected:
  ADReal computeStrongResidual() override;

  /// dimension
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// harmonic interpolation?
  const bool _harmonic_interpolation;
};

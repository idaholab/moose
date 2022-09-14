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
#include "INSFVMomentumResidualObject.h"

class INSFVMomentumDiffusionComplete : public INSFVMomentumDiffusion
{
public:
  static InputParameters validParams();
  INSFVMomentumDiffusionComplete(const InputParameters & params);

protected:
  /**
   * Routine to compute this object's strong residual (e.g. not multipled by area). This routine
   * should also populate the _ae and _an coefficients
   */
  virtual ADReal computeStrongResidual();

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

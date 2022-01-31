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
#include "INSFVBCInterface.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class INSFVAdvectionKernel : public FVFluxKernel, public INSFVBCInterface
{
public:
  static InputParameters validParams();
  INSFVAdvectionKernel(const InputParameters & params);
  void initialSetup() override;

protected:
  bool skipForBoundary(const FaceInfo & fi) const override;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;

  /// The Rhie-Chow user object that provides us with the velocity
  const INSFVRhieChowInterpolator & _rc_vel_provider;
};

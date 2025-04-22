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

class FVAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVAdvection(const InputParameters & params);

  /**
   * Helper function to set the relationship manager parameters
   * used for proper ghosting setup. Needed, because the required ghosting
   * depends on the interpolation parameters supplied to this object.
   */
  static void setRMParams(const InputParameters & obj_params,
    InputParameters & rm_params);

protected:
  virtual ADReal computeQpResidual() override;

  const RealVectorValue _velocity;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumPressure.h"

/**
 * Introduces the coupled pressure term into the Navier-Stokes porous media momentum equation.
 */
class PINSFVMomentumPressure : public INSFVMomentumPressure
{
public:
  static InputParameters validParams();
  PINSFVMomentumPressure(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the porosity
  const VariableValue & _eps;
  /// the porosity as a variable to be able to compute gradients
  const MooseVariableFV<Real> * const _eps_var;
  /// whether the porosity has no discontinuities
  const bool _smooth_porosity;
  /// maximum porosity gradient norm before considering a discontinuity exists (only if smooth_porosity is false)
  const Real _max_eps_gradient;
};

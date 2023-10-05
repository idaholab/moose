//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

class INSFVRhieChowInterpolator;

/**
 * Corrects energy advection for a non-constant specific heat capacity
 */
class INSFVEnergyHeatCapacityGradient : public FVElementalKernel
{
public:
  static InputParameters validParams();

  INSFVEnergyHeatCapacityGradient(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The gradient of the constant pressure specific heat capacity
  const Moose::Functor<ADRealVectorValue> & _grad_cp;

  /// The density
  const Moose::Functor<ADReal> & _rho;

  /// The Rhie-Chow user object we will query for the velocity
  const INSFVRhieChowInterpolator & _rc_vel_provider;
};

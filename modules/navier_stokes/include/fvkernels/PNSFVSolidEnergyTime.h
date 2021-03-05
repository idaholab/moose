//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

/**
 * Kernel providing the time derivative term in the solid energy equation,
 * with strong form $(1-\epsilon)C_{p,s}\frac{\partial T_s}{\partial t}$.
 */
class PNSFVSolidEnergyTime : public FVTimeKernel
{
public:
  PNSFVSolidEnergyTime(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  /// Porosity
  const MaterialProperty<Real> & _eps;

  /// scales the value of the kernel, used for faster steady state during pseudo transient
  const Real & _scaling;

  /// whether a zero scaling factor has been specifed
  const bool _zero_scaling;

  /// Solid density
  const ADMaterialProperty<Real> * const _rho_s;

  /// Solid isobaric specific heat
  const ADMaterialProperty<Real> * const _cp_s;
};

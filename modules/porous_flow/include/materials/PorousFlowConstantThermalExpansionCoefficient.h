//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Material designed to provide a time-invariant
 * volumetric thermal expansion coefficient
 * A = * (alpha - phi) * alT + phi * alF .
 * Here
 * alpha = Biot coefficient (assumed constant)
 * phi = initial value of porosity
 * alT = drained volumetric thermal expansion coefficient (assumed constant)
 * alF = fluid volumetric thermal expansion coefficient (assumed constant)
 */
class PorousFlowConstantThermalExpansionCoefficient : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowConstantThermalExpansionCoefficient(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Biot coefficient
  const Real _biot_coefficient;

  /// Fluid volumetric thermal expansion coefficient
  const Real _fluid_coefficient;

  /// Drained porous-skeleton volumetric thermal expansion coefficient
  const Real _drained_coefficient;

  /// porosity at the nodes or quadpoints.  Only the initial value is ever used
  const MaterialProperty<Real> & _porosity;

  /// Computed volumetric thermal expansion coefficient
  MaterialProperty<Real> & _coeff;

  /// Old value of the volumetric thermal expansion coefficient.  This variable is necessary in order to keep the thermal expansion coefficient constant even if porosity is changing.
  const MaterialProperty<Real> & _coeff_old;
};

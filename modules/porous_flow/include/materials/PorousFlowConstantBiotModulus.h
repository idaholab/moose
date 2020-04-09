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
 * Biot Modulus, M, where
 * 1 / M = (1 - alpha) * (alpha - phi) * C + phi / Kf .
 * Here
 * alpha = Biot coefficient (assumed constant)
 * phi = initial value of porosity
 * C = drained porous-solid bulk compliance (1 / bulk modulus)
 * Kf = fluid bulk modulus (assumed constant)
 */
class PorousFlowConstantBiotModulus : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowConstantBiotModulus(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Biot coefficient
  const Real _biot_coefficient;

  /// Fluid bulk modulus
  const Real _fluid_bulk_modulus;

  /// Solid bulk compliance
  const Real _solid_bulk_compliance;

  /// porosity at the nodes or quadpoints.  Only the initial value is ever used
  const MaterialProperty<Real> & _porosity;

  /// Computed Biot modulus
  MaterialProperty<Real> & _biot_modulus;

  /// Old value of Biot modulus.  This variable is necessary in order to keep Biot modulus constant even if porosity is changing.
  const MaterialProperty<Real> & _biot_modulus_old;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class SinglePhaseFluidProperties;

/**
 * Computes dynamic viscosity
 */
class ADDynamicViscosityMaterial : public Material
{
public:
  ADDynamicViscosityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Dynamic viscosity name
  const MaterialPropertyName & _mu_name;

  // Dynamic viscosity
  ADMaterialProperty<Real> & _mu;

  /// Specific volume
  const ADMaterialProperty<Real> & _v;

  /// Specific internal energy
  const ADMaterialProperty<Real> & _e;

  /// Single-phase fluid properties
  const SinglePhaseFluidProperties & _fp_1phase;

public:
  static InputParameters validParams();
};

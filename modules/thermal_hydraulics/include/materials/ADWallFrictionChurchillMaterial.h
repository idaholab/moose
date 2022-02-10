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

/**
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class ADWallFrictionChurchillMaterial : public Material
{
public:
  ADWallFrictionChurchillMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Darcy wall friction coefficient
  const MaterialPropertyName _f_D_name;
  ADMaterialProperty<Real> & _f_D;

  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _mu;

  /// Density of the phase
  const ADMaterialProperty<Real> & _rho;
  /// Velocity (x-component)
  const ADMaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;
  /// Roughness of the surface
  const Real & _roughness;

public:
  static InputParameters validParams();
};

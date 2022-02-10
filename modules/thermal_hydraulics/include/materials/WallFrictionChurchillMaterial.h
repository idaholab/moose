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
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class WallFrictionChurchillMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallFrictionChurchillMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Darcy wall friction coefficient
  const MaterialPropertyName _f_D_name;
  MaterialProperty<Real> & _f_D;
  MaterialProperty<Real> & _df_D_drhoA;
  MaterialProperty<Real> & _df_D_drhouA;
  MaterialProperty<Real> & _df_D_drhoEA;

  /// Dynamic viscosity
  const MaterialProperty<Real> & _mu;

  /// Density of the phase
  const MaterialProperty<Real> & _rho;
  /// Velocity (x-component)
  const MaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Roughness of the surface
  const Real & _roughness;

public:
  static InputParameters validParams();
};

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "MooseEnum.h"

/**
 * Computes drag coefficient using the Colebrook-White formula for the Darcy friction factor
 */
class ADWallFrictionColebrookWhiteMaterial : public Material
{
public:
  ADWallFrictionColebrookWhiteMaterial(const InputParameters & parameters);

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

  /// max iterations for iterative solve
  const unsigned int _max_its;
  /// Whether to error, warn or accept on reaching max its
  MooseEnum _max_its_behavior;
  /// Tolerance for implicit solve
  const Real _tol;

public:
  static InputParameters validParams();
};

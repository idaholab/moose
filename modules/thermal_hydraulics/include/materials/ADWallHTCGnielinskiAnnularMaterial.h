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
 * Computes wall heat transfer coefficient for gases and water in an annular flow
 * channel using the Gnielinski correlation
 */
class ADWallHTCGnielinskiAnnularMaterial : public Material
{
public:
  ADWallHTCGnielinskiAnnularMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Wall heat transfer coefficient
  ADMaterialProperty<Real> & _htc_wall;
  /// Density
  const ADMaterialProperty<Real> & _rho;
  /// Velocity
  const ADMaterialProperty<Real> & _vel;
  /// Thermal conductivity
  const ADMaterialProperty<Real> & _k;
  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _mu;
  /// Specific heat capacity
  const ADMaterialProperty<Real> & _cp;
  /// Pressure
  const ADMaterialProperty<Real> & _p;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// Wall temperature
  const ADMaterialProperty<Real> & _T_wall;
  /// Inner diameter
  const Real & _D_inner;
  /// Outer diameter
  const Real & _D_outer;
  /// Hydraulic diameter
  const Real _D_h;
  /// Diameter ratio
  const Real _a;
  /// Channel length
  const Real & _L;
  /// Heat transfer occurs at inner wall
  const bool _at_inner_wall;
  /// Fluid is a gas?
  const bool _fluid_is_gas;
  /// Gas heating correction exponent
  const Real & _n;
  /// Provided gas heating correction exponent?
  const bool _provided_gas_heating_correction_exponent;
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};

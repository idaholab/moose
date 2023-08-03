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
 * Computes drag coefficient using the Cheng-Todreas correlation for Fanning friction factor
 */
class ADWallFrictionChengMaterial : public Material
{
public:
  ADWallFrictionChengMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Rod bundle array type
  enum class Bundle_array
  {
    SQUARE,
    HEXAGONAL,
  };
  /// Subchannel type:
  enum class Subchannel_type
  {
    INTERIOR,
    EDGE,
    CORNER,
  };

  /// Darcy wall friction coefficient
  const MaterialPropertyName _f_D_name;
  ADMaterialProperty<Real> & _f_D;
  /// Density
  const ADMaterialProperty<Real> & _rho;
  /// Velocity
  const ADMaterialProperty<Real> & _vel;
  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;
  /// Dynamic viscosity
  const ADMaterialProperty<Real> & _mu;
  /// Pitch-to-Diameter ratio
  const Real & _PoD;
  /// Rod bundle array type
  const Bundle_array _bundle_array;
  /// Subchannel type
  const Subchannel_type _subchannel_type;

public:
  static InputParameters validParams();
};

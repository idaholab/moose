//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Computes drag coefficient using the Cheng-Todreas correlation for Fanning friction factor
 */
class ADWallFrictionChengFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADWallFrictionChengFunctorMaterial(const InputParameters & parameters);

protected:
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

  /// Darcy wall friction coefficient name
  const MooseFunctorName _f_D_name;
  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Velocity
  const Moose::Functor<ADReal> & _vel;
  /// Hydraulic diameter
  const Moose::Functor<ADReal> & _D_h;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Pitch-to-Diameter ratio
  const Real & _PoD;
  /// Rod bundle array type
  const Bundle_array _bundle_array;
  /// Subchannel type
  const Subchannel_type _subchannel_type;
};

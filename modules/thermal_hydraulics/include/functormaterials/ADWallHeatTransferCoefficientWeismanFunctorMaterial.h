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
 * Computes wall heat transfer coefficient for liquid sodium using Schad-modified correlation
 */
class ADWallHeatTransferCoefficientWeismanFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADWallHeatTransferCoefficientWeismanFunctorMaterial(const InputParameters & parameters);

protected:
  /// Rod bundle array type
  enum class Bundle_array
  {
    SQUARE,
    TRIANGULAR,
  };

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Velocity
  const Moose::Functor<ADReal> & _vel;
  /// Hydraulic diameter
  const Moose::Functor<ADReal> & _D_h;
  /// Thermal conductivity
  const Moose::Functor<ADReal> & _k;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Specific heat capacity
  const Moose::Functor<ADReal> & _cp;
  /// Fluid temperature
  const Moose::Functor<ADReal> & _T;
  /// Wall temperature
  const Moose::Functor<ADReal> & _T_wall;
  /// Pitch-to-Diameter ratio
  const Real & _PoD;
  /// Rod bundle array type
  const Bundle_array _bundle_array;
};

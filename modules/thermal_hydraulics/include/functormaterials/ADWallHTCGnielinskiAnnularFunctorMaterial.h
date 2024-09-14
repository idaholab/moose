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

class SinglePhaseFluidProperties;

/**
 * Computes wall heat transfer coefficient for gases and water in an annular flow
 * channel using the Gnielinski correlation
 */
class ADWallHTCGnielinskiAnnularFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADWallHTCGnielinskiAnnularFunctorMaterial(const InputParameters & parameters);

protected:
  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Velocity
  const Moose::Functor<ADReal> & _vel;
  /// Thermal conductivity
  const Moose::Functor<ADReal> & _k;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Specific heat capacity
  const Moose::Functor<ADReal> & _cp;
  /// Pressure
  const Moose::Functor<ADReal> & _p;
  /// Fluid temperature
  const Moose::Functor<ADReal> & _T;
  /// Wall temperature
  const Moose::Functor<ADReal> & _T_wall;
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
};

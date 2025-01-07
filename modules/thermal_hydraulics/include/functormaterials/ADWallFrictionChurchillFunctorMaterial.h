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
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class ADWallFrictionChurchillFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADWallFrictionChurchillFunctorMaterial(const InputParameters & parameters);

protected:
  /// Darcy wall friction coefficient name
  const MooseFunctorName _f_D_name;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Density of the phase
  const Moose::Functor<ADReal> & _rho;
  /// Velocity (x-component)
  const Moose::Functor<ADReal> & _vel;
  /// Hydraulic diameter
  const Moose::Functor<ADReal> & _D_h;
  /// Roughness of the surface
  const Real & _roughness;
};

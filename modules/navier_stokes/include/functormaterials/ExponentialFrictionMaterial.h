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
 * Class responsible for generating a friction factor for the
 * friction-based pressure loss terms in the form of:
 *
 * $f(Re) = C_1 Re^{C_2}$
 *
 * This is common for flows in pipes. Designed to work with both
 * INSFV and PINSFV friction loss kernels.
 */
class ExponentialFrictionMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  ExponentialFrictionMaterial(const InputParameters & parameters);

private:
  /// Functor for the Reynolds number
  const Moose::Functor<ADReal> & _Re;
  /// Speed (velocity magnitude) of the fluid
  const Moose::Functor<ADReal> * const _speed;
  /// $C_1$ in $f(Re) = C_1 Re^{C_2}$
  const Real _c1;
  /// $C_2$ in $f(Re) = C_1 Re^{C_2}$
  const Real _c2;
  /// The name of the output friction factor functor
  const std::string _friction_factor_name;
  /// If a factor of velocity should be included or not
  const bool _include_velocity_factor;
};

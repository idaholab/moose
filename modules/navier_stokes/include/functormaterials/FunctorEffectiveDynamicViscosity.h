//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Class used to compute the effective dynamic viscosity, notably in the presence of turbulence
 */
template <bool is_ad>
class FunctorEffectiveDynamicViscosityTempl : public FunctorMaterial
{
public:
  FunctorEffectiveDynamicViscosityTempl(const InputParameters & parameters);

  static InputParameters validParams();

  /// Functor for the dynamic viscosity
  const Moose::Functor<GenericReal<is_ad>> & _mu;
  /// Functor for the turbulent dynamic viscosity
  const Moose::Functor<GenericReal<is_ad>> & _mu_t;
  /// Functor for dividing the turbulent dynamic viscosity
  const Moose::Functor<GenericReal<is_ad>> & _scale_factor;
  /// Factor for dividing the turbulent dynamic viscosity
  Real _scale_factor_real;
};

typedef FunctorEffectiveDynamicViscosityTempl<false> FunctorEffectiveDynamicViscosity;
typedef FunctorEffectiveDynamicViscosityTempl<true> ADFunctorEffectiveDynamicViscosity;

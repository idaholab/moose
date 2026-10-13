//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunction.h"

/**
 * Test function \f$ f(t) = a + b t \f$ providing a value and a time derivative. Objects evaluating
 * functions of temperature pass the temperature as the time argument, so this function exercises
 * their derivative terms.
 */
class KokkosLinearTimeFunction : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();

  KokkosLinearTimeFunction(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;

  KOKKOS_FUNCTION Real value(Real t, Real3 /* p */) const { return _a + _b * t; }
  KOKKOS_FUNCTION Real timeDerivative(Real /* t */, Real3 /* p */) const { return _b; }

private:
  /// Constant term
  const Real _a;
  /// Linear coefficient
  const Real _b;
};

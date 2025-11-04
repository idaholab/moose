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
 * Class that represents constant function
 */
class KokkosConstantFunction : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();

  KokkosConstantFunction(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;

  KOKKOS_FUNCTION Real value(Real /* t */, Real3 /* p */) const { return _value; }
  KOKKOS_FUNCTION Real timeIntegral(Real t1, Real t2, Real3 /* p */) const
  {
    return _value * (t2 - t1);
  }

protected:
  Moose::Kokkos::Scalar<const Real> _value;
};

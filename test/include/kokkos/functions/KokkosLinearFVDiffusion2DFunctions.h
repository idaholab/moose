//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunction.h"

class KokkosLinearFV2DExactSolution : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();
  KokkosLinearFV2DExactSolution(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;
  KOKKOS_FUNCTION Real value(Real, Real3 p) const
  {
    return (1.5 - p(0) * p(0)) * (1.5 - p(1) * p(1));
  }
};

class KokkosLinearFV2DDiffusionCoefficient : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();
  KokkosLinearFV2DDiffusionCoefficient(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;
  KOKKOS_FUNCTION Real value(Real, Real3 p) const { return 1.0 + 0.5 * p(0) * p(1); }
};

class KokkosLinearFV2DSourceFunction : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();
  KokkosLinearFV2DSourceFunction(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;
  KOKKOS_FUNCTION Real value(Real, Real3 p) const
  {
    return 2.0 * (1.5 - p(1) * p(1)) + 2.0 * p(0) * p(1) * (1.5 - p(1) * p(1)) +
           2.0 * (1.5 - p(0) * p(0)) + 2.0 * p(0) * p(1) * (1.5 - p(0) * p(0));
  }
};

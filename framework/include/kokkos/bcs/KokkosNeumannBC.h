//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"

class KokkosNeumannBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int, AssemblyDatum &) const;

protected:
  /// Value of grad(u) on the boundary.
  const Real _value;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosNeumannBC::computeQpResidual(const unsigned int, AssemblyDatum &) const
{
  return -_value;
}

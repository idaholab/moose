//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADIntegratedBC.h"

class KokkosADNeumannBC : public Moose::Kokkos::ADIntegratedBC
{
public:
  static InputParameters validParams();

  KokkosADNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::ADReal
  computeQpResidual(const unsigned int i, const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Value of grad(u) on the boundary.
  const Real _value;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::ADReal
KokkosADNeumannBC::computeQpResidual(const unsigned int i,
                                     const unsigned int qp,
                                     AssemblyDatum & datum) const
{
  return -_test(datum, i, qp) * _value;
}

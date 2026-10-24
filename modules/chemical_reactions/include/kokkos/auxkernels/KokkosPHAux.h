//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Kokkos pH of the solution
 */
class KokkosPHAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosPHAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Free molar concentration of H+ ions
  const Moose::Kokkos::VariableValue _hplus;
  /// Activity coefficient of H+ ions
  const Moose::Kokkos::VariableValue _gamma;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPHAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real activity = _gamma(datum, qp) * _hplus(datum, qp);

  KOKKOS_ASSERT(activity > 0.0);

  return -::Kokkos::log10(activity);
}

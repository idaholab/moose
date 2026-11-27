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
 * Kokkos total concentration of a primary species including its stoichiometric contribution to
 * secondary equilibrium species
 */
class KokkosTotalConcentrationAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosTotalConcentrationAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Primary species free concentration
  const Moose::Kokkos::VariableValue _primary_species;
  /// Stoichiometric coefficients of the primary species in the secondary equilibrium species
  Moose::Kokkos::Array<Real> _sto_v;
  /// Number of secondary equilibrium species
  const unsigned int _n;
  /// Secondary equilibrium species concentrations
  const Moose::Kokkos::VariableValue _secondary_species;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTotalConcentrationAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  Real total_concentration = _primary_species(datum, qp);

  for (unsigned int i = 0; i < _n; ++i)
    total_concentration += _sto_v[i] * _secondary_species(datum, qp, i);

  return total_concentration;
}

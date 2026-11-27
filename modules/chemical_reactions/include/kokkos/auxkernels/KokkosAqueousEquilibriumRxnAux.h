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
 * Kokkos concentration of a secondary equilibrium species
 */
class KokkosAqueousEquilibriumRxnAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosAqueousEquilibriumRxnAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Activity coefficient of a coupled species
  KOKKOS_FUNCTION Real gammaV(AssemblyDatum & datum,
                              const unsigned int qp,
                              const unsigned int i) const
  {
    return _gamma_v(datum, qp, _gamma_v_coupled ? i : 0);
  }

  /// Equilibrium constant
  const Moose::Kokkos::VariableValue _log_k;
  /// Stoichiometric coefficients of the coupled primary species
  Moose::Kokkos::Array<Real> _sto_v;
  /// Activity coefficient of the equilibrium species
  const Moose::Kokkos::VariableValue _gamma_eq;
  /// Number of coupled primary species
  const unsigned int _n;
  /// Coupled primary species concentrations
  const Moose::Kokkos::VariableValue _vals;
  /// Whether the activity coefficients of the coupled species are coupled variables
  const bool _gamma_v_coupled;
  /// Activity coefficients of the coupled primary species
  const Moose::Kokkos::VariableValue _gamma_v;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosAqueousEquilibriumRxnAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  Real conc_product = 1.0;

  for (unsigned int i = 0; i < _n; ++i)
    conc_product *= ::Kokkos::pow(gammaV(datum, qp, i) * _vals(datum, qp, i), _sto_v[i]);

  KOKKOS_ASSERT(_gamma_eq(datum, qp) > 0.0);

  return ::Kokkos::pow(10.0, _log_k(datum, qp)) * conc_product / _gamma_eq(datum, qp);
}

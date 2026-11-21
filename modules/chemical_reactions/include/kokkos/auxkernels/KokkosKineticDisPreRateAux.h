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
 * Kokkos kinetic rate of a secondary kinetic species
 */
class KokkosKineticDisPreRateAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosKineticDisPreRateAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return kineticRate(qp, datum);
  }

protected:
  /// Kinetic rate of the species
  KOKKOS_FUNCTION Real kineticRate(const unsigned int qp, AssemblyDatum & datum) const;

  /// Equilibrium constant
  const Moose::Kokkos::VariableValue _log_k;
  /// Specific reactive surface area
  const Real _r_area;
  /// Reference kinetic rate constant
  const Real _ref_kconst;
  /// Activation energy
  const Real _e_act;
  /// Gas constant
  const Real _gas_const;
  /// Reference temperature
  const Real _ref_temp;
  /// System temperature
  const Moose::Kokkos::VariableValue _sys_temp;
  /// Stoichiometric coefficients of the reactant species
  Moose::Kokkos::Array<Real> _sto_v;
  /// Number of reactant species
  const unsigned int _n;
  /// Reactant species concentrations
  const Moose::Kokkos::VariableValue _vals;
};

KOKKOS_FUNCTION inline Real
KokkosKineticDisPreRateAux::kineticRate(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real kconst =
      _ref_kconst *
      ::Kokkos::exp(_e_act * (1.0 / _ref_temp - 1.0 / _sys_temp(datum, qp)) / _gas_const);

  Real omega = 1.0;

  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real val = _vals(datum, qp, i);

    if (val < 0.0)
      omega *= 0.0;
    else
      omega *= ::Kokkos::pow(val, _sto_v[i]);
  }

  const Real saturation_SI = omega / ::Kokkos::pow(10.0, _log_k(datum, qp));
  Real kinetic_rate = _r_area * kconst * (1.0 - saturation_SI);

  // Rates below this threshold are treated as zero, as in KineticDisPreRateAux
  if (::Kokkos::abs(kinetic_rate) <= 1.0e-12)
    kinetic_rate = 0.0;

  return -kinetic_rate;
}

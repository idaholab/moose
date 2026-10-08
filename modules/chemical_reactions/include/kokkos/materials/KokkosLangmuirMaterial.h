//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Material type that holds info regarding Langmuir desorption from matrix to porespace and
 * viceversa
 */
class KokkosLangmuirMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosLangmuirMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Reciprocal of the desorption time constant
  const Moose::Kokkos::VariableValue _one_over_de_time_const;
  /// Reciprocal of the adsorption time constant
  const Moose::Kokkos::VariableValue _one_over_ad_time_const;
  /// Langmuir density
  const Real _langmuir_dens;
  /// Langmuir pressure
  const Real _langmuir_p;
  /// Concentration of the adsorbed gas in the matrix
  const Moose::Kokkos::VariableValue _conc;
  /// Gas porepressure
  const Moose::Kokkos::VariableValue _pressure;
  /// Mass flow rate from the matrix to the porespace
  Moose::Kokkos::MaterialProperty<Real> _mass_rate_from_matrix;
  /// Derivative of the mass flow rate with respect to the concentration
  Moose::Kokkos::MaterialProperty<Real> _dmass_rate_from_matrix_dC;
  /// Derivative of the mass flow rate with respect to the porepressure
  Moose::Kokkos::MaterialProperty<Real> _dmass_rate_from_matrix_dp;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosLangmuirMaterial::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const Real conc = _conc(datum, qp);
  const Real p = _pressure(datum, qp);

  const Real equilib_conc = _langmuir_dens * p / (_langmuir_p + p);
  const Real dequilib_conc_dp = _langmuir_dens / (_langmuir_p + p) -
                                _langmuir_dens * p / ((_langmuir_p + p) * (_langmuir_p + p));

  // form the base rate and derivs without the appropriate time const
  Real rate = conc - equilib_conc;
  Real drate_dC = 1.0;
  Real drate_dp = -dequilib_conc_dp;

  // multiply by the appropriate time const
  const Real one_over_time_const =
      conc > equilib_conc ? _one_over_de_time_const(datum, qp) : _one_over_ad_time_const(datum, qp);

  _mass_rate_from_matrix(datum, qp) = rate * one_over_time_const;
  _dmass_rate_from_matrix_dC(datum, qp) = drate_dC * one_over_time_const;
  _dmass_rate_from_matrix_dp(datum, qp) = drate_dp * one_over_time_const;
}

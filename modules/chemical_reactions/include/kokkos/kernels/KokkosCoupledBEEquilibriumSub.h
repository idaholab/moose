//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeKernelValue.h"
#include "KokkosMap.h"

/**
 * Kokkos derivative of the equilibrium species concentration with respect to time, discretized
 * with the backward Euler method
 */
class KokkosCoupledBEEquilibriumSub : public Moose::Kokkos::TimeKernelValue
{
public:
  static InputParameters validParams();

  KokkosCoupledBEEquilibriumSub(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpOffDiagJacobian(const unsigned int j,
                                                   const unsigned int jvar,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const;

private:
  /// Activity coefficient of a coupled species
  KOKKOS_FUNCTION Real gammaV(AssemblyDatum & datum,
                              const unsigned int qp,
                              const unsigned int i) const
  {
    return _gamma_v(datum, qp, _gamma_v_coupled ? i : 0);
  }
  /// Old activity coefficient of a coupled species
  KOKKOS_FUNCTION Real gammaVOld(AssemblyDatum & datum,
                                 const unsigned int qp,
                                 const unsigned int i) const
  {
    return _gamma_v_old(datum, qp, _gamma_v_coupled ? i : 0);
  }

  /// Weight of the equilibrium species in the total concentration
  const Real _weight;
  /// Equilibrium constant of the equilibrium species
  const Moose::Kokkos::VariableValue _log_k;
  /// Stoichiometric coefficient of the primary species
  const Real _sto_u;
  /// Stoichiometric coefficients of the coupled primary species
  Moose::Kokkos::Array<Real> _sto_v;
  /// Activity coefficient of the primary species
  const Moose::Kokkos::VariableValue _gamma_u;
  /// Old activity coefficient of the primary species
  const Moose::Kokkos::VariableValue _gamma_u_old;
  /// Whether the activity coefficients of the coupled species are coupled variables
  const bool _gamma_v_coupled;
  /// Activity coefficients of the coupled primary species
  const Moose::Kokkos::VariableValue _gamma_v;
  /// Old activity coefficients of the coupled primary species
  const Moose::Kokkos::VariableValue _gamma_v_old;
  /// Activity coefficient of the equilibrium species
  const Moose::Kokkos::VariableValue _gamma_eq;
  /// Old activity coefficient of the equilibrium species
  const Moose::Kokkos::VariableValue _gamma_eq_old;
  /// Porosity
  const Moose::Kokkos::MaterialProperty<Real> _porosity;
  /// Variable numbers of the coupled primary species
  const std::vector<unsigned int> _vars;
  /// Map from the coupled variable numbers to the coupled species indices
  Moose::Kokkos::Map<unsigned int, unsigned int> _var_to_index;
  /// Number of coupled primary species
  const unsigned int _n;
  /// Coupled primary species concentrations
  const Moose::Kokkos::VariableValue _v_vals;
  /// Old coupled primary species concentrations
  const Moose::Kokkos::VariableValue _v_vals_old;
  /// Old value of the primary species
  const Moose::Kokkos::VariableValue _u_old;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledBEEquilibriumSub::precomputeQpResidual(const unsigned int qp,
                                                    AssemblyDatum & datum) const
{
  KOKKOS_ASSERT(_gamma_eq(datum, qp) > 0.0);

  const Real k = ::Kokkos::pow(10.0, _log_k(datum, qp));

  // Contribution due to primary species that this kernel acts on
  Real val_new =
      k * ::Kokkos::pow(_gamma_u(datum, qp) * _u(datum, qp), _sto_u) / _gamma_eq(datum, qp);
  Real val_old = k * ::Kokkos::pow(_gamma_u_old(datum, qp) * _u_old(datum, qp), _sto_u) /
                 _gamma_eq_old(datum, qp);

  // Contribution due to coupled primary species
  for (unsigned int i = 0; i < _n; ++i)
  {
    val_new *= ::Kokkos::pow(gammaV(datum, qp, i) * _v_vals(datum, qp, i), _sto_v[i]);
    val_old *= ::Kokkos::pow(gammaVOld(datum, qp, i) * _v_vals_old(datum, qp, i), _sto_v[i]);
  }

  return _porosity(datum, qp) * _weight * (val_new - val_old) / _dt;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledBEEquilibriumSub::precomputeQpJacobian(const unsigned int j,
                                                    const unsigned int qp,
                                                    AssemblyDatum & datum) const
{
  const Real gamma_u = _gamma_u(datum, qp);

  Real val_new = ::Kokkos::pow(10.0, _log_k(datum, qp)) * _sto_u * gamma_u *
                 ::Kokkos::pow(gamma_u * _u(datum, qp), _sto_u - 1.0) * _phi(datum, j, qp) /
                 _gamma_eq(datum, qp);

  for (unsigned int i = 0; i < _n; ++i)
    val_new *= ::Kokkos::pow(gammaV(datum, qp, i) * _v_vals(datum, qp, i), _sto_v[i]);

  return _porosity(datum, qp) * _weight * val_new / _dt;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledBEEquilibriumSub::precomputeQpOffDiagJacobian(const unsigned int j,
                                                           const unsigned int jvar,
                                                           const unsigned int qp,
                                                           AssemblyDatum & datum) const
{
  // If jvar is not one of the coupled species, return 0
  if (!_var_to_index.exists(jvar))
    return 0;

  const auto ivar = _var_to_index[jvar];

  Real val_new = ::Kokkos::pow(10.0, _log_k(datum, qp)) *
                 ::Kokkos::pow(_gamma_u(datum, qp) * _u(datum, qp), _sto_u) / _gamma_eq(datum, qp);

  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real gamma_v = gammaV(datum, qp, i);

    if (i == ivar)
      val_new *= _sto_v[i] * gamma_v *
                 ::Kokkos::pow(gamma_v * _v_vals(datum, qp, i), _sto_v[i] - 1.0) *
                 _phi(datum, j, qp);
    else
      val_new *= ::Kokkos::pow(gamma_v * _v_vals(datum, qp, i), _sto_v[i]);
  }

  return _porosity(datum, qp) * _weight * val_new / _dt;
}

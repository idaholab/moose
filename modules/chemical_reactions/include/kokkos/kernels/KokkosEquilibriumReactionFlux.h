//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"
#include "KokkosMap.h"

/**
 * Base class for the Kokkos kernels transporting an equilibrium species formed by the primary
 * species of the kernel variable and coupled primary species. It computes the gradient of the
 * equilibrium species concentration and its derivatives in the same arithmetic order as the
 * original objects.
 *
 * In the products over the other coupled species, the activity coefficient of the current species
 * multiplies the concentrations of the other species. This matches the original objects.
 */
template <typename Base>
class KokkosEquilibriumReactionFlux : public Base
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosEquilibriumReactionFlux(const InputParameters & parameters);

protected:
  /// Gradient of the equilibrium species concentration without the equilibrium constant factor
  KOKKOS_FUNCTION Real3 flux(const unsigned int qp, Moose::Kokkos::AssemblyDatum & datum) const;
  /// Derivative of flux() with respect to the kernel variable
  KOKKOS_FUNCTION Real3 fluxJacobian(const unsigned int j,
                                     const unsigned int qp,
                                     Moose::Kokkos::AssemblyDatum & datum) const;
  /// Derivative of flux() with respect to the coupled species with index ivar
  KOKKOS_FUNCTION Real3 fluxOffDiagJacobian(const unsigned int j,
                                            const unsigned int ivar,
                                            const unsigned int qp,
                                            Moose::Kokkos::AssemblyDatum & datum) const;
  /// Factor multiplying flux(): weight * 10^log_k / gamma_eq
  KOKKOS_FUNCTION Real factor(const unsigned int qp, Moose::Kokkos::AssemblyDatum & datum) const;

  /// Activity coefficient of a coupled species
  KOKKOS_FUNCTION Real gammaV(Moose::Kokkos::AssemblyDatum & datum,
                              const unsigned int qp,
                              const unsigned int i) const
  {
    return _gamma_v(datum, qp, _gamma_v_coupled ? i : 0);
  }

  /// Weight of the equilibrium species
  const Real _weight;
  /// Equilibrium constant of the equilibrium species
  const Moose::Kokkos::VariableValue _log_k;
  /// Stoichiometric coefficient of the primary species
  const Real _sto_u;
  /// Stoichiometric coefficients of the coupled primary species
  Moose::Kokkos::Array<Real> _sto_v;
  /// Variable numbers of the coupled primary species
  const std::vector<unsigned int> _vars;
  /// Map from the coupled variable numbers to the coupled species indices
  Moose::Kokkos::Map<unsigned int, unsigned int> _var_to_index;
  /// Number of coupled primary species
  const unsigned int _n;
  /// Coupled primary species concentrations
  const Moose::Kokkos::VariableValue _vals;
  /// Coupled primary species concentration gradients
  const Moose::Kokkos::VariableGradient _grad_vals;
  /// Activity coefficient of the primary species
  const Moose::Kokkos::VariableValue _gamma_u;
  /// Whether the activity coefficients of the coupled species are coupled variables
  const bool _gamma_v_coupled;
  /// Activity coefficients of the coupled primary species
  const Moose::Kokkos::VariableValue _gamma_v;
  /// Activity coefficient of the equilibrium species
  const Moose::Kokkos::VariableValue _gamma_eq;
};

template <typename Base>
KOKKOS_FUNCTION Real
KokkosEquilibriumReactionFlux<Base>::factor(const unsigned int qp,
                                            Moose::Kokkos::AssemblyDatum & datum) const
{
  KOKKOS_ASSERT(_gamma_eq(datum, qp) > 0.0);

  return _weight * ::Kokkos::pow(10.0, _log_k(datum, qp)) / _gamma_eq(datum, qp);
}

template <typename Base>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEquilibriumReactionFlux<Base>::flux(const unsigned int qp,
                                          Moose::Kokkos::AssemblyDatum & datum) const
{
  const Real gamma_u = _gamma_u(datum, qp);
  const Real u = this->_u(datum, qp);

  Real3 diff1 =
      _sto_u * gamma_u * ::Kokkos::pow(gamma_u * u, _sto_u - 1.0) * this->_grad_u(datum, qp);
  for (unsigned int i = 0; i < _n; ++i)
    diff1 *= ::Kokkos::pow(gammaV(datum, qp, i) * _vals(datum, qp, i), _sto_v[i]);

  Real3 diff2_sum(0);
  const Real d_val = ::Kokkos::pow(gamma_u * u, _sto_u);
  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real gamma_v = gammaV(datum, qp, i);
    Real3 diff2 = d_val * _sto_v[i] * gamma_v *
                  ::Kokkos::pow(gamma_v * _vals(datum, qp, i), _sto_v[i] - 1.0) *
                  _grad_vals(datum, qp, i);
    for (unsigned int k = 0; k < _n; ++k)
      if (k != i)
        diff2 *= ::Kokkos::pow(gamma_v * _vals(datum, qp, k), _sto_v[k]);
    diff2_sum += diff2;
  }

  return diff1 + diff2_sum;
}

template <typename Base>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEquilibriumReactionFlux<Base>::fluxJacobian(const unsigned int j,
                                                  const unsigned int qp,
                                                  Moose::Kokkos::AssemblyDatum & datum) const
{
  const Real gamma_u = _gamma_u(datum, qp);
  const Real u = this->_u(datum, qp);
  const Real phi = this->_phi(datum, j, qp);

  Real3 diff1_1 =
      _sto_u * gamma_u * ::Kokkos::pow(gamma_u * u, _sto_u - 1.0) * this->_grad_phi(datum, j, qp);
  Real3 diff1_2 = phi * _sto_u * (_sto_u - 1.0) * gamma_u * gamma_u *
                  ::Kokkos::pow(gamma_u * u, _sto_u - 2.0) * this->_grad_u(datum, qp);
  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real product = ::Kokkos::pow(gammaV(datum, qp, i) * _vals(datum, qp, i), _sto_v[i]);
    diff1_1 *= product;
    diff1_2 *= product;
  }

  const Real d_val = _sto_u * gamma_u * ::Kokkos::pow(gamma_u * u, _sto_u - 1.0) * phi;
  Real3 diff2_sum(0);
  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real gamma_v = gammaV(datum, qp, i);
    Real3 diff2 = d_val * _sto_v[i] * gamma_v *
                  ::Kokkos::pow(gamma_v * _vals(datum, qp, i), _sto_v[i] - 1.0) *
                  _grad_vals(datum, qp, i);
    for (unsigned int k = 0; k < _n; ++k)
      if (k != i)
        diff2 *= ::Kokkos::pow(gamma_v * _vals(datum, qp, k), _sto_v[k]);
    diff2_sum += diff2;
  }

  return diff1_1 + diff1_2 + diff2_sum;
}

template <typename Base>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEquilibriumReactionFlux<Base>::fluxOffDiagJacobian(const unsigned int j,
                                                         const unsigned int ivar,
                                                         const unsigned int qp,
                                                         Moose::Kokkos::AssemblyDatum & datum) const
{
  const Real gamma_u = _gamma_u(datum, qp);
  const Real u = this->_u(datum, qp);
  const Real phi = this->_phi(datum, j, qp);

  Real3 diff1 =
      _sto_u * gamma_u * ::Kokkos::pow(gamma_u * u, _sto_u - 1.0) * this->_grad_u(datum, qp);
  for (unsigned int i = 0; i < _n; ++i)
  {
    const Real gamma_v = gammaV(datum, qp, i);
    if (i == ivar)
      diff1 *=
          _sto_v[i] * gamma_v * ::Kokkos::pow(gamma_v * _vals(datum, qp, i), _sto_v[i] - 1.0) * phi;
    else
      diff1 *= ::Kokkos::pow(gamma_v * _vals(datum, qp, i), _sto_v[i]);
  }

  const Real val_u = ::Kokkos::pow(gamma_u * u, _sto_u);
  const Real gamma_j = gammaV(datum, qp, ivar);
  const Real val_j = gamma_j * _vals(datum, qp, ivar);

  const Real3 diff2_1 = _sto_v[ivar] * (_sto_v[ivar] - 1.0) * gamma_j * gamma_j *
                        ::Kokkos::pow(val_j, _sto_v[ivar] - 2.0) * phi *
                        _grad_vals(datum, qp, ivar);
  const Real3 diff2_2 = _sto_v[ivar] * gamma_j * ::Kokkos::pow(val_j, _sto_v[ivar] - 1.0) *
                        this->_grad_phi(datum, j, qp);

  Real3 diff2 = val_u * (diff2_1 + diff2_2);
  for (unsigned int i = 0; i < _n; ++i)
    if (i != ivar)
      diff2 *= ::Kokkos::pow(gammaV(datum, qp, i) * _vals(datum, qp, i), _sto_v[i]);

  const Real val_jvar =
      val_u * _sto_v[ivar] * gamma_j * ::Kokkos::pow(val_j, _sto_v[ivar] - 1.0) * phi;

  Real3 diff3_sum(0);
  for (unsigned int i = 0; i < _n; ++i)
    if (i != ivar)
    {
      const Real gamma_v = gammaV(datum, qp, i);
      Real3 diff3 = val_jvar * _sto_v[i] * gamma_v *
                    ::Kokkos::pow(gamma_v * _vals(datum, qp, i), _sto_v[i] - 1.0) *
                    _grad_vals(datum, qp, i);
      for (unsigned int k = 0; k < _n; ++k)
        if (k != ivar && k != i)
          diff3 *= ::Kokkos::pow(gamma_v * _vals(datum, qp, k), _sto_v[k]);
      diff3_sum += diff3;
    }

  return diff1 + diff2 + diff3_sum;
}

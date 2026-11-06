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
 * Conductivity k(T) = k0 (1 + beta T) of a coupled variable T, with its derivative provided as an
 * on-demand property
 */
class KokkosLinearConductivity : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosLinearConductivity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Variable the conductivity depends on
  const Moose::Kokkos::VariableValue _T;
  /// Conductivity at T = 0
  const Real _k0;
  /// Linear coefficient of the conductivity in T
  const Real _beta;

  /// Conductivity
  Moose::Kokkos::MaterialProperty<Real> _k;
  /// Derivative of the conductivity with respect to T, allocated only when consumed
  Moose::Kokkos::MaterialProperty<Real> _dk_dT;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosLinearConductivity::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  _k(datum, qp) = _k0 * (1 + _beta * _T(datum, qp));

  // Only allocated when another object consumes it
  if (_dk_dT)
    _dk_dT(datum, qp) = _k0 * _beta;
}

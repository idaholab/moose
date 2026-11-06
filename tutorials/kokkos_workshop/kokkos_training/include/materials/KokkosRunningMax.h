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
 * Stateful property holding the maximum of a coupled variable over all time steps so far
 */
class KokkosRunningMax : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosRunningMax(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Variable whose maximum is tracked
  const Moose::Kokkos::VariableValue _v;
  /// Running maximum
  Moose::Kokkos::MaterialProperty<Real> _max;
  /// Running maximum at the previous time step
  Moose::Kokkos::MaterialProperty<Real> _max_old;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosRunningMax::initQpStatefulProperties(const unsigned int qp, Datum & datum) const
{
  _max(datum, qp) = _v(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosRunningMax::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const Real old_max = _max_old(datum, qp);
  _max(datum, qp) = ::Kokkos::max(old_max, _v(datum, qp));
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosHeatConduction.h"

/**
 * Kokkos heat conduction kernel for truss elements, taking the cross-sectional area into account
 */
class KokkosTrussHeatConduction : public KokkosHeatConduction
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosTrussHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Cross-sectional area of the truss element
  const Moose::Kokkos::VariableValue _area;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosTrussHeatConduction::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _area(datum, qp) * KokkosHeatConduction::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosTrussHeatConduction::precomputeQpJacobian(const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return _area(datum, qp) * KokkosHeatConduction::precomputeQpJacobian<Derived>(j, qp, datum);
}

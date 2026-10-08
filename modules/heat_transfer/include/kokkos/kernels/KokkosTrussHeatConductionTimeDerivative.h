//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosHeatConductionTimeDerivative.h"

/**
 * Kokkos heat conduction time derivative kernel for truss elements, taking the cross-sectional
 * area into account
 */
class KokkosTrussHeatConductionTimeDerivative : public KokkosHeatConductionTimeDerivative
{
public:
  static InputParameters validParams();

  KokkosTrussHeatConductionTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Cross-sectional area of the truss element
  const Moose::Kokkos::VariableValue _area;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTrussHeatConductionTimeDerivative::precomputeQpResidual(const unsigned int qp,
                                                              AssemblyDatum & datum) const
{
  return _area(datum, qp) *
         KokkosHeatConductionTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTrussHeatConductionTimeDerivative::precomputeQpJacobian(const unsigned int j,
                                                              const unsigned int qp,
                                                              AssemblyDatum & datum) const
{
  return _area(datum, qp) *
         KokkosHeatConductionTimeDerivative::precomputeQpJacobian<Derived>(j, qp, datum);
}

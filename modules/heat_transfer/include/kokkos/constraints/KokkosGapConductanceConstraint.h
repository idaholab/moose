//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMortarConstraint.h"

/**
 * Thermal contact through a gap of constant conductance with a Lagrange multiplier, the heat flux
 * through the gap: lambda = k (T_primary - T_secondary) / l, with l the gap width bounded below by
 * a minimum gap
 */
class KokkosGapConductanceConstraint : public Moose::Kokkos::MortarConstraint
{
public:
  static InputParameters validParams();

  KokkosGapConductanceConstraint(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const MortarSide side,
                                         const Moose::Kokkos::MortarDatum & datum) const;
  KOKKOS_FUNCTION void computeQpJacobian(const MortarSide side,
                                         const Moose::Kokkos::MortarDatum & datum,
                                         Real & d_secondary,
                                         Real & d_primary,
                                         Real & d_lambda) const;

private:
  /// The gap width at a point, bounded below by the minimum gap
  KOKKOS_FUNCTION Real gapWidth(const Moose::Kokkos::MortarDatum & datum) const;

  /// Thermal conductivity of the gap medium
  const Real _k;
  /// Minimum gap width
  const Real _min_gap;
};

KOKKOS_FUNCTION inline Real
KokkosGapConductanceConstraint::gapWidth(const Moose::Kokkos::MortarDatum & datum) const
{
  return datum.gap > _min_gap ? datum.gap : _min_gap;
}

KOKKOS_FUNCTION inline Real
KokkosGapConductanceConstraint::computeQpResidual(const MortarSide side,
                                                  const Moose::Kokkos::MortarDatum & datum) const
{
  switch (side)
  {
    case MortarSide::Secondary:
      return -datum.lambda;
    case MortarSide::Primary:
      return datum.lambda;
    case MortarSide::Lower:
      return datum.lambda - _k * (datum.u_primary - datum.u_secondary) / gapWidth(datum);
  }

  return 0;
}

KOKKOS_FUNCTION inline void
KokkosGapConductanceConstraint::computeQpJacobian(const MortarSide side,
                                                  const Moose::Kokkos::MortarDatum & datum,
                                                  Real & d_secondary,
                                                  Real & d_primary,
                                                  Real & d_lambda) const
{
  switch (side)
  {
    case MortarSide::Secondary:
      d_lambda = -1;
      break;
    case MortarSide::Primary:
      d_lambda = 1;
      break;
    case MortarSide::Lower:
      d_lambda = 1;
      d_primary = -_k / gapWidth(datum);
      d_secondary = _k / gapWidth(datum);
      break;
  }
}

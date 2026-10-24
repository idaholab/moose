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
 * Constrain the value of a variable to be the same on both sides of an interface with a Lagrange
 * multiplier, the flux through the interface
 */
class KokkosEqualValueConstraint : public Moose::Kokkos::MortarConstraint
{
public:
  static InputParameters validParams();

  KokkosEqualValueConstraint(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const MortarSide side,
                                         const Moose::Kokkos::MortarDatum & datum) const;
  KOKKOS_FUNCTION void computeQpJacobian(const MortarSide side,
                                         const Moose::Kokkos::MortarDatum & datum,
                                         Real & d_secondary,
                                         Real & d_primary,
                                         Real & d_lambda,
                                         Real & d_gap) const;
};

KOKKOS_FUNCTION inline Real
KokkosEqualValueConstraint::computeQpResidual(const MortarSide side,
                                              const Moose::Kokkos::MortarDatum & datum) const
{
  switch (side)
  {
    // The sign choice makes lambda = normals_secondary * grad_u_secondary for the true solution
    case MortarSide::Secondary:
      return -datum.lambda;
    case MortarSide::Primary:
      return datum.lambda;
    case MortarSide::Lower:
      return datum.u_primary - datum.u_secondary;
  }

  return 0;
}

KOKKOS_FUNCTION inline void
KokkosEqualValueConstraint::computeQpJacobian(const MortarSide side,
                                              const Moose::Kokkos::MortarDatum &,
                                              Real & d_secondary,
                                              Real & d_primary,
                                              Real & d_lambda,
                                              Real &) const
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
      d_primary = 1;
      d_secondary = -1;
      break;
  }
}

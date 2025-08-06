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

/**
 *  Implements a simple consuming reaction term with weak form $(\\psi_i, \\lambda u_h)$.
 */
class KokkosReaction final : public Moose::Kokkos::Kernel<KokkosReaction>
{
public:
  static InputParameters validParams();

  KokkosReaction(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

protected:
  /// Scalar coefficient representing the relative amount consumed per unit time
  const Moose::Kokkos::Scalar<const Real> _rate;
};

KOKKOS_FUNCTION inline Real
KokkosReaction::computeQpResidual(const unsigned int i,
                                  const unsigned int qp,
                                  ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _rate * _u(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosReaction::computeQpJacobian(const unsigned int i,
                                  const unsigned int j,
                                  const unsigned int qp,
                                  ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _rate * _phi(datum, j, qp);
}

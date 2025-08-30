//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

/**
 * Implements a simple Vacuum BC for neutron diffusion on the boundary.
 * Vacuum BC is defined as \f$ D\frac{du}{dn}+\frac{u}{2} = 0\f$, where u is neutron flux.
 * Hence, \f$ D\frac{du}{dn}=-\frac{u}{2} \f$ and \f$ -\frac{u}{2} \f$ is substituted into
 * the Neumann BC term produced from integrating the diffusion operator by parts.
 */
class KokkosVacuumBC final : public Moose::Kokkos::IntegratedBC<KokkosVacuumBC>
{
public:
  static InputParameters validParams();

  KokkosVacuumBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;

private:
  /// Ratio of u to du/dn
  const Real _alpha;
};

KOKKOS_FUNCTION inline Real
KokkosVacuumBC::computeQpResidual(const unsigned int i,
                                  const unsigned int qp,
                                  ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _alpha * _u(datum, qp) / 2.;
}

KOKKOS_FUNCTION inline Real
KokkosVacuumBC::computeQpJacobian(const unsigned int i,
                                  const unsigned int j,
                                  const unsigned int qp,
                                  ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _alpha * _phi(datum, j, qp) / 2.;
}

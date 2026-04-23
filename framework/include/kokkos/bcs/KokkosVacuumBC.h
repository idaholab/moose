//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"

/**
 * Implements a simple Vacuum BC for neutron diffusion on the boundary.
 * Vacuum BC is defined as \f$ D\frac{du}{dn}+\frac{u}{2} = 0\f$, where u is neutron flux.
 * Hence, \f$ D\frac{du}{dn}=-\frac{u}{2} \f$ and \f$ -\frac{u}{2} \f$ is substituted into
 * the Neumann BC term produced from integrating the diffusion operator by parts.
 */
class KokkosVacuumBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosVacuumBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  /// Ratio of u to du/dn
  const Real _alpha;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVacuumBC::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _alpha * _u(datum, qp) / 2.;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVacuumBC::computeQpJacobian(const unsigned int j,
                                  const unsigned int qp,
                                  AssemblyDatum & datum) const
{
  return _alpha * _phi(datum, j, qp) / 2.;
}

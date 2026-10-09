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
 * Nonlinear diffusion -div(k(u) grad(u)) with the conductivity k(u) = k0 (1 + beta u)
 */
class KokkosNonlinearDiffusion : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosNonlinearDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  /// Conductivity at u = 0
  const Real _k0;
  /// Linear coefficient of the conductivity in u
  const Real _beta;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosNonlinearDiffusion::computeQpResidual(const unsigned int i,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const
{
  const Real k = _k0 * (1 + _beta * _u(datum, qp));
  return k * _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosNonlinearDiffusion::computeQpJacobian(const unsigned int i,
                                            const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const
{
  const Real k = _k0 * (1 + _beta * _u(datum, qp));
  return (k * _grad_phi(datum, j, qp) + _k0 * _beta * _phi(datum, j, qp) * _grad_u(datum, qp)) *
         _grad_test(datum, i, qp);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"

/**
 * KernelGrad version of KokkosNonlinearDiffusion: the flux k(u) grad(u) is computed once per
 * quadrature point and multiplied by the test function gradients by the base class
 */
class KokkosNonlinearDiffusionGrad : public Moose::Kokkos::KernelGrad
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosNonlinearDiffusionGrad(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Conductivity at u = 0
  const Real _k0;
  /// Linear coefficient of the conductivity in u
  const Real _beta;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosNonlinearDiffusionGrad::precomputeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  return _k0 * (1 + _beta * _u(datum, qp)) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosNonlinearDiffusionGrad::precomputeQpJacobian(const unsigned int j,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  return _k0 * (1 + _beta * _u(datum, qp)) * _grad_phi(datum, j, qp) +
         _k0 * _beta * _phi(datum, j, qp) * _grad_u(datum, qp);
}

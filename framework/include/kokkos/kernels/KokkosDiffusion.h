//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
template <typename Derived>
class KokkosDiffusion : public Moose::Kokkos::Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams();

  KokkosDiffusion(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;
};

template <typename Derived>
InputParameters
KokkosDiffusion<Derived>::validParams()
{
  InputParameters params = Moose::Kokkos::Kernel<Derived>::validParams();
  return params;
}

template <typename Derived>
KokkosDiffusion<Derived>::KokkosDiffusion(const InputParameters & parameters)
  : Moose::Kokkos::Kernel<Derived>(parameters)
{
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosDiffusion<Derived>::computeQpResidual(const unsigned int i,
                                            const unsigned int qp,
                                            ResidualDatum & datum) const
{
  return _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosDiffusion<Derived>::computeQpJacobian(const unsigned int i,
                                            const unsigned int j,
                                            const unsigned int qp,
                                            ResidualDatum & datum) const
{
  return _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}

class KokkosDiffusionKernel final : public KokkosDiffusion<KokkosDiffusionKernel>
{
public:
  static InputParameters validParams();

  KokkosDiffusionKernel(const InputParameters & parameters);
};

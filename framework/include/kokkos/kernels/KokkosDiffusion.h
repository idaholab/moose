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
class KokkosDiffusion : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosDiffusion(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
};

KOKKOS_FUNCTION inline Real
KokkosDiffusion::computeQpResidual(const unsigned int i,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const
{
  return _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

KOKKOS_FUNCTION inline Real
KokkosDiffusion::computeQpJacobian(const unsigned int i,
                                   const unsigned int j,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const
{
  return _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}

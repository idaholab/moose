//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernelGrad.h"

/**
 * This vector kernel implements the Laplacian operator:
 * $\nabla \vec{u} : \nabla \vec{\phi_i}$
 */
class KokkosVectorDiffusion : public Moose::Kokkos::VectorKernelGrad
{
public:
  static InputParameters validParams();

  KokkosVectorDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpResidualGrad(const unsigned int qp,
                                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpJacobianGrad(const unsigned int j,
                                                                 const unsigned int qp,
                                                                 AssemblyDatum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorDiffusion::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorDiffusion::precomputeQpJacobianGrad(const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return _grad_phi(datum, j, qp);
}

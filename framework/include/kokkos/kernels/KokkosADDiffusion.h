//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernel.h"

/**
 * The Laplacian operator $(\nabla \psi_i, \nabla u_h)$ with its Jacobian computed by pointwise
 * automatic differentiation
 */
class KokkosADDiffusion : public Moose::Kokkos::ADKernel
{
public:
  /// The residual depends on grad u
  static constexpr unsigned int max_ad_slots = 3;

  static InputParameters validParams();

  KokkosADDiffusion(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal3<slots>
  precomputeQpResidualGrad(const unsigned int qp,
                           Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    return _grad_u(datum, qp);
  }

private:
  /// Gradient of the kernel variable
  const Moose::Kokkos::ADVariableGradient _grad_u;
};

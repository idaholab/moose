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
 * The diffusion operator $(\nabla \psi_i, D \nabla u_h)$ with a diffusivity given by a pointwise AD
 * material property, whose derivatives with respect to the inputs of its material enter the
 * Jacobian computed by pointwise automatic differentiation
 */
class KokkosADMatDiffusion : public Moose::Kokkos::ADKernel
{
public:
  /// grad u, followed by the inputs of the diffusivity coupled to u: one coupled field value and
  /// gradient besides the value of u
  static constexpr unsigned int max_ad_slots = 8;

  static InputParameters validParams();

  KokkosADMatDiffusion(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal3<slots>
  precomputeQpResidualGrad(const unsigned int qp,
                           Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    return _diffusivity(datum, qp) * _grad_u(datum, qp);
  }

private:
  /// Gradient of the kernel variable
  const Moose::Kokkos::ADVariableGradient _grad_u;
  /// Diffusivity
  const Moose::Kokkos::ADMaterialPropertyReader _diffusivity;
};

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
 * Kokkos kernel for asymptotic expansion homogenization of an anisotropic thermal conductivity
 */
class KokkosAnisoHomogenizedHeatConduction : public Moose::Kokkos::KernelGrad
{
public:
  static InputParameters validParams();

  KokkosAnisoHomogenizedHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidual(const unsigned int qp,
                                                            AssemblyDatum & datum) const;

private:
  /// Diffusion coefficient tensor
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _diffusion_coefficient;
  /// Direction the variable of this kernel acts in
  const unsigned int _component;
  /// Mesh dimension
  const unsigned int _dim;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosAnisoHomogenizedHeatConduction::precomputeQpResidual(const unsigned int qp,
                                                           AssemblyDatum & datum) const
{
  // This is the matrix-vector product of the diffusion coefficient tensor with
  // the j-th unit vector
  const Moose::Kokkos::Real33 d = _diffusion_coefficient(datum, qp);
  Moose::Kokkos::Real3 d_times_ej(0);
  for (unsigned int j = 0; j < _dim; ++j)
    d_times_ej(j) = d(j, _component);
  return d_times_ej;
}

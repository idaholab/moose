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
 * Kokkos kernel for asymptotic expansion homogenization of the thermal conductivity
 */
class KokkosHomogenizedHeatConduction : public Moose::Kokkos::KernelGrad
{
public:
  static InputParameters validParams();

  KokkosHomogenizedHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidual(const unsigned int qp,
                                                            AssemblyDatum & datum) const;

private:
  /// Diffusion coefficient
  const Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient;
  /// Direction the variable of this kernel acts in
  const unsigned int _component;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHomogenizedHeatConduction::precomputeQpResidual(const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  // Compute positive value since we are computing a residual not a rhs
  Moose::Kokkos::Real3 k_times_e(0);
  k_times_e(_component) = _diffusion_coefficient(datum, qp);
  return k_times_e;
}

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
 * Kokkos diffusion of the primary species
 */
class KokkosPrimaryDiffusion : public Moose::Kokkos::KernelGrad
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosPrimaryDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Dispersion-diffusion coefficient
  const Moose::Kokkos::MaterialProperty<Real> _diffusivity;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosPrimaryDiffusion::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _diffusivity(datum, qp) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosPrimaryDiffusion::precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
{
  return _diffusivity(datum, qp) * _grad_phi(datum, j, qp);
}

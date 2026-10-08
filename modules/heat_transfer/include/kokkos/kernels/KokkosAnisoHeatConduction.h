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
#include "DerivativeMaterialInterface.h"

/**
 * Kokkos kernel for a heat conduction term with an anisotropic conductivity tensor
 */
class KokkosAnisoHeatConduction : public DerivativeMaterialInterface<Moose::Kokkos::KernelGrad>
{
  using Real3 = Moose::Kokkos::Real3;
  using Real33 = Moose::Kokkos::Real33;

public:
  static InputParameters validParams();

  KokkosAnisoHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Thermal conductivity tensor
  const Moose::Kokkos::MaterialProperty<Real33> _k;
  /// Derivative of the thermal conductivity tensor with respect to temperature
  const Moose::Kokkos::MaterialProperty<Real33> _dk_dT;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosAnisoHeatConduction::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real33 k = _k(datum, qp);
  return k * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosAnisoHeatConduction::precomputeQpJacobian(const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  const Real33 k = _k(datum, qp);
  const Real33 dk_dT = _dk_dT(datum, qp);
  return k * _grad_phi(datum, j, qp) + dk_dT * (_phi(datum, j, qp) * _grad_u(datum, qp));
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosEquilibriumReactionFlux.h"
#include "KokkosKernelGrad.h"

/**
 * Kokkos diffusion of an equilibrium species
 */
class KokkosCoupledDiffusionReactionSub
  : public KokkosEquilibriumReactionFlux<Moose::Kokkos::KernelGrad>
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosCoupledDiffusionReactionSub(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpOffDiagJacobian(const unsigned int j,
                                                    const unsigned int jvar,
                                                    const unsigned int qp,
                                                    AssemblyDatum & datum) const;

private:
  /// Diffusivity
  const Moose::Kokkos::MaterialProperty<Real> _diffusivity;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosCoupledDiffusionReactionSub::precomputeQpResidual(const unsigned int qp,
                                                        AssemblyDatum & datum) const
{
  return factor(qp, datum) * _diffusivity(datum, qp) * flux(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosCoupledDiffusionReactionSub::precomputeQpJacobian(const unsigned int j,
                                                        const unsigned int qp,
                                                        AssemblyDatum & datum) const
{
  return factor(qp, datum) * _diffusivity(datum, qp) * fluxJacobian(j, qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosCoupledDiffusionReactionSub::precomputeQpOffDiagJacobian(const unsigned int j,
                                                               const unsigned int jvar,
                                                               const unsigned int qp,
                                                               AssemblyDatum & datum) const
{
  // If jvar is not one of the coupled species, return 0
  if (!_var_to_index.exists(jvar))
    return Real3(0);

  return factor(qp, datum) * _diffusivity(datum, qp) *
         fluxOffDiagJacobian(j, _var_to_index[jvar], qp, datum);
}

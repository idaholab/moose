//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernelGrad.h"

/**
 * Diffusion with the solution-dependent conductivity $1 + u^2$, whose linearization occupies both
 * flux blocks of the quadrature-point Jacobian tensor: the flux depends on the solution value
 * through the conductivity and on the solution gradient through the gradient it multiplies.
 */
class KokkosADNonlinearDiffusion : public Moose::Kokkos::ADKernelGrad
{
public:
  static InputParameters validParams();

  KokkosADNonlinearDiffusion(const InputParameters & parameters);

  template <typename T>
  KOKKOS_FUNCTION Moose::Kokkos::Vector3<T> computeQpFlux(const T & u,
                                                          const Moose::Kokkos::Vector3<T> & grad_u,
                                                          const unsigned int qp,
                                                          AssemblyDatum & datum) const;
};

template <typename T>
KOKKOS_FUNCTION Moose::Kokkos::Vector3<T>
KokkosADNonlinearDiffusion::computeQpFlux(const T & u,
                                          const Moose::Kokkos::Vector3<T> & grad_u,
                                          const unsigned int /* qp */,
                                          AssemblyDatum & /* datum */) const
{
  return (1.0 + u * u) * grad_u;
}

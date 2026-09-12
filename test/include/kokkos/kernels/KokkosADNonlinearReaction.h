//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernelValue.h"

/**
 * The reaction-like term $u^2 + \nabla u \cdot \nabla u$, whose linearization occupies both value
 * blocks of the quadrature-point Jacobian tensor.
 */
class KokkosADNonlinearReaction : public Moose::Kokkos::ADKernelValue
{
public:
  static InputParameters validParams();

  KokkosADNonlinearReaction(const InputParameters & parameters);

  template <typename T>
  KOKKOS_FUNCTION T computeQpValue(const T & u,
                                   const Moose::Kokkos::Vector3<T> & grad_u,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const;
};

template <typename T>
KOKKOS_FUNCTION T
KokkosADNonlinearReaction::computeQpValue(const T & u,
                                          const Moose::Kokkos::Vector3<T> & grad_u,
                                          const unsigned int /* qp */,
                                          AssemblyDatum & /* datum */) const
{
  return u * u + grad_u * grad_u;
}

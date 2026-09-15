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

class KokkosADDiffusion : public Moose::Kokkos::ADKernelGrad
{
public:
  static InputParameters validParams();

  KokkosADDiffusion(const InputParameters & parameters);

  template <typename T>
  KOKKOS_FUNCTION Moose::Kokkos::Vector3<T> computeQpFlux(const T & u,
                                                          const Moose::Kokkos::Vector3<T> & grad_u,
                                                          const unsigned int qp,
                                                          AssemblyDatum & datum) const;
};

template <typename T>
KOKKOS_FUNCTION Moose::Kokkos::Vector3<T>
KokkosADDiffusion::computeQpFlux(const T & /* u */,
                                 const Moose::Kokkos::Vector3<T> & grad_u,
                                 const unsigned int /* qp */,
                                 AssemblyDatum & /* datum */) const
{
  return grad_u;
}

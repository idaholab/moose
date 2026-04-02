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

class KokkosADDiffusion : public Moose::Kokkos::ADKernel
{
public:
  static InputParameters validParams();

  KokkosADDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::ADReal
  computeQpResidual(const unsigned int i, const unsigned int qp, AssemblyDatum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::ADReal
KokkosADDiffusion::computeQpResidual(const unsigned int i,
                                     const unsigned int qp,
                                     AssemblyDatum & datum) const
{
  return _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

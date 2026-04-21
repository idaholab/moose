//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADTimeKernel.h"

class KokkosADTimeDerivative : public Moose::Kokkos::ADTimeKernel
{
public:
  static InputParameters validParams();

  KokkosADTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::ADReal
  computeQpResidual(const unsigned int i, const unsigned int qp, AssemblyDatum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::ADReal
KokkosADTimeDerivative::computeQpResidual(const unsigned int i,
                                          const unsigned int qp,
                                          AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _u_dot(datum, qp);
}

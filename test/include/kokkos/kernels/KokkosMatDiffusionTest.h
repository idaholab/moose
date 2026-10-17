//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

class KokkosMatDiffusionTest : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosMatDiffusionTest(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualGrad(const unsigned int qp,
                                                                AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianGrad(const unsigned int j,
                                                                const unsigned int qp,
                                                                AssemblyDatum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _diff;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosMatDiffusionTest::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return _diff(datum, qp) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosMatDiffusionTest::precomputeQpJacobianGrad(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  return _diff(datum, qp) * _grad_phi(datum, j, qp);
}

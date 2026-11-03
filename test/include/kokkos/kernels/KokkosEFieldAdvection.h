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

class KokkosEFieldAdvection : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosEFieldAdvection(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualGrad(const unsigned int qp,
                                                                AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianGrad(const unsigned int j,
                                                                const unsigned int qp,
                                                                AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                                                       const unsigned int jvar,
                                                                       const unsigned int qp,
                                                                       AssemblyDatum & datum) const;

protected:
  const unsigned int _efield_id;
  const Moose::Kokkos::VectorVariableValue _efield;
  const bool _efield_coupled;
  const Moose::Kokkos::VectorVariablePhiValue _vector_phi;
  const Real _mobility;
  const Real _sgn;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEFieldAdvection::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return -_sgn * _mobility * _efield(datum, qp) * _u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEFieldAdvection::precomputeQpJacobianGrad(const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return -_sgn * _mobility * _efield(datum, qp) * _phi(datum, j, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosEFieldAdvection::precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                                       const unsigned int jvar,
                                                       const unsigned int qp,
                                                       AssemblyDatum & datum) const
{
  return jvar == _efield_id && _efield_coupled
             ? -_sgn * _mobility * _vector_phi(datum, j, qp) * _u(datum, qp)
             : Moose::Kokkos::Real3(0);
}

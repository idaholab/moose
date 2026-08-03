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
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
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
KOKKOS_FUNCTION Real
KokkosEFieldAdvection::computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const
{
  return -_sgn * _mobility * (_grad_test(datum, i, qp) * _efield(datum, qp)) * _u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosEFieldAdvection::computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const
{
  return -_sgn * _mobility * (_grad_test(datum, i, qp) * _efield(datum, qp)) * _phi(datum, j, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosEFieldAdvection::computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return jvar == _efield_id && _efield_coupled
             ? -_sgn * _mobility * (_grad_test(datum, i, qp) * _vector_phi(datum, j, qp)) *
                   _u(datum, qp)
             : 0;
}

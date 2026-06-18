//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernel.h"

class KokkosCoupledVectorDiffusion : public Moose::Kokkos::VectorKernel
{
public:
  static InputParameters validParams();

  KokkosCoupledVectorDiffusion(const InputParameters & parameters);

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
  const bool _current_state;
  const Moose::Kokkos::VectorVariableGradient _grad_v;
  const unsigned int _v_id;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledVectorDiffusion::computeQpResidual(const unsigned int i,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return -_grad_v(datum, qp).contract(_grad_test(datum, i, qp));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledVectorDiffusion::computeQpJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
{
  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledVectorDiffusion::computeQpOffDiagJacobian(const unsigned int i,
                                                       const unsigned int j,
                                                       const unsigned int jvar,
                                                       const unsigned int qp,
                                                       AssemblyDatum & datum) const
{
  return jvar == _v_id && _current_state
             ? -_grad_phi(datum, j, qp).contract(_grad_test(datum, i, qp))
             : 0;
}

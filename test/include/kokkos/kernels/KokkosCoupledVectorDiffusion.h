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
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpResidualGrad(const unsigned int qp,
                                                                 AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpJacobianGrad(const unsigned int /* j */,
                                                                 const unsigned int /* qp */,
                                                                 AssemblyDatum & /* datum */) const;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33
  precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                  const unsigned int jvar,
                                  const unsigned int qp,
                                  AssemblyDatum & datum) const;

protected:
  const bool _current_state;
  const Moose::Kokkos::VectorVariableGradient _grad_v;
  const unsigned int _v_id;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosCoupledVectorDiffusion::precomputeQpResidualGrad(const unsigned int qp,
                                                       AssemblyDatum & datum) const
{
  return -1.0 * _grad_v(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosCoupledVectorDiffusion::precomputeQpJacobianGrad(const unsigned int /* j */,
                                                       const unsigned int /* qp */,
                                                       AssemblyDatum & /* datum */) const
{
  return Moose::Kokkos::Real33(0);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosCoupledVectorDiffusion::precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                                              const unsigned int jvar,
                                                              const unsigned int qp,
                                                              AssemblyDatum & datum) const
{
  return jvar == _v_id && _current_state ? -1.0 * _grad_phi(datum, j, qp)
                                         : Moose::Kokkos::Real33(0);
}

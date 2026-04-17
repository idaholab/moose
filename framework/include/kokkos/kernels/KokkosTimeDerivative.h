//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeKernel.h"

class KokkosTimeDerivative : public Moose::Kokkos::TimeKernel
{
public:
  static InputParameters validParams();

  KokkosTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  const bool _lumping;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosTimeDerivative::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  using Moose::Kokkos::MAX_CACHED_DOF;

  Real local_ke[MAX_CACHED_DOF];

  for (unsigned int j = datum.local_thread_id(); j < datum.n_jdofs();
       j += datum.num_local_threads())
  {
    unsigned int num_batches = datum.n_idofs() / MAX_CACHED_DOF;

    if (datum.n_idofs() % MAX_CACHED_DOF)
      ++num_batches;

    for (unsigned int batch = 0; batch < num_batches; ++batch)
    {
      unsigned int ib = batch * MAX_CACHED_DOF;
      unsigned int ie = ::Kokkos::min(ib + MAX_CACHED_DOF, datum.n_idofs());

      for (unsigned int i = ib; i < ie; ++i)
        local_ke[i - ib] = 0;

      for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        for (unsigned int i = ib; i < ie; ++i)
          local_ke[i - ib] +=
              datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(i, j, qp, datum);

      for (unsigned int i = ib; i < ie; ++i)
        accumulateTaggedElementalMatrix(
            local_ke[i - ib], datum.elem().id, i, _lumping ? i : j, datum.jvar());
    }
  }
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::computeQpResidual(const unsigned int i,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _u_dot(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::computeQpJacobian(const unsigned int i,
                                        const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _phi(datum, j, qp) * _du_dot_du;
}

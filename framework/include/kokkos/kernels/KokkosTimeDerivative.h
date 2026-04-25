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
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobianDummy(const unsigned int,
                                              const unsigned int,
                                              AssemblyDatum &) const
  {
    return 0;
  }

  // The computeQpJacobian() of this class has the same signature with that of KernelValue (without
  // test function index), but this class itself derives from TimeKernel whose base class is Kernel
  // (with test function index). Therefore, their function pointers cannot be compared. Instead, we
  // provide the function pointer of a dummy function to the dispatcher registry to make it think
  // that non-default computeQpJacobian() was implemented.
  template <typename Derived>
  static auto defaultJacobian()
  {
    return &KokkosTimeDerivative::computeQpJacobianDummy<Derived>;
  }

protected:
  const bool _lumping;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosTimeDerivative::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real value = datum.JxW(qp) * kernel.template computeQpResidual<Derived>(qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _test(datum, i, qp);
        }
      });
}

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
      {
        Real value = datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(j, qp, datum);

        for (unsigned int i = ib; i < ie; ++i)
          local_ke[i - ib] += value * _test(datum, i, qp);
      }

      for (unsigned int i = ib; i < ie; ++i)
        accumulateTaggedElementalMatrix(
            local_ke[i - ib], datum.elem().id, i, _lumping ? i : j, datum.jvar());
    }
  }
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _u_dot(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::computeQpJacobian(const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _phi(datum, j, qp) * _du_dot_du;
}

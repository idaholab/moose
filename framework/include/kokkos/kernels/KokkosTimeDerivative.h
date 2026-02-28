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

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
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

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_CACHED_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_CACHED_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_CACHED_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_CACHED_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int ij = ijb; ij < ije; ++ij)
      {
        unsigned int i = ij % datum.n_jdofs();
        unsigned int j = ij / datum.n_jdofs();

        local_ke[ij - ijb] += datum.JxW(qp) * kernel.computeQpJacobianShim(kernel, i, j, qp, datum);
      }
    }

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(
          local_ke[ij - ijb], datum.elem().id, i, _lumping ? i : j, datum.jvar(), datum.comp());
    }
  }
}

KOKKOS_FUNCTION inline Real
KokkosTimeDerivative::computeQpResidual(const unsigned int i,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _u_dot(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosTimeDerivative::computeQpJacobian(const unsigned int i,
                                        const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _phi(datum, j, qp) * _du_dot_du[datum.comp()];
}

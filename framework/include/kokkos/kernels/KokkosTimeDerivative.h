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
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] +=
                datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(i, j, qp, datum);
      },
      _lumping);
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

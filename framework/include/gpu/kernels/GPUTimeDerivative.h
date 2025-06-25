//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTimeKernel.h"

template <typename Derived>
class KokkosTimeDerivative : public Moose::Kokkos::TimeKernel<Derived>
{
  usingKokkosTimeKernelMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = Moose::Kokkos::TimeKernel<Derived>::validParams();
    params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
    return params;
  }

  KokkosTimeDerivative::KokkosTimeDerivative(const InputParameters & parameters)
    : Moose::Kokkos::TimeKernel<Derived>(parameters),
      _lumping(this->template getParam<bool>("lumping"))
  {
  }

  KOKKOS_FUNCTION void
  computeJacobianInternal(const Derived * kernel, ResidualDatum & datum, Real * local_ke) const
  {
    if (_lumping)
    {
      for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
      {
        datum.reinit(qp);

        for (unsigned int i = 0; i < datum.n_idofs(); ++i)
          for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
            local_ke[i] += datum.JxW(qp) * kernel->computeQpJacobian(i, j, qp, datum);
      }

      for (unsigned int i = 0; i < datum.n_idofs(); ++i)
        accumulateTaggedLocalMatrix(local_ke[i], datum.elem().id, i, i, datum.jvar());
    }
    else
      Moose::Kokkos::TimeKernel<Derived>::computeJacobianInternal(kernel, datum, local_ke);
  }

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _test(datum, i, qp) * _u_dot(datum, qp);
  }
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _test(datum, i, qp) * _phi(datum, j, qp) * _du_dot_du;
  }

protected:
  const bool _lumping;
};

class KokkosTimeDerivativeKernel final : public KokkosTimeDerivative<KokkosTimeDerivativeKernel>
{
public:
  static InputParameters validParams();

  KokkosTimeDerivativeKernel(const InputParameters & parameters);
};

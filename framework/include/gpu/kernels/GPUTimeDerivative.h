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
  static InputParameters validParams();

  KokkosTimeDerivative(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const;

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

protected:
  const bool _lumping;
};

template <typename Derived>
InputParameters
KokkosTimeDerivative<Derived>::validParams()
{
  InputParameters params = Moose::Kokkos::TimeKernel<Derived>::validParams();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
  return params;
}

template <typename Derived>
KokkosTimeDerivative<Derived>::KokkosTimeDerivative(const InputParameters & parameters)
  : Moose::Kokkos::TimeKernel<Derived>(parameters),
    _lumping(this->template getParam<bool>("lumping"))
{
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosTimeDerivative<Derived>::computeJacobianInternal(const Derived * kernel,
                                                       ResidualDatum & datum) const
{
  using Moose::Kokkos::MAX_DOF;

  Real local_ke[MAX_DOF];

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int ij = ijb; ij < ije; ++ij)
      {
        unsigned int i = ij % datum.n_jdofs();
        unsigned int j = ij / datum.n_jdofs();

        local_ke[ij - ijb] += datum.JxW(qp) * kernel->computeQpJacobian(i, j, qp, datum);
      }
    }

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(
          local_ke[ij - ijb], datum.elem().id, i, _lumping ? i : j, datum.jvar());
    }
  }
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosTimeDerivative<Derived>::computeQpResidual(const unsigned int i,
                                                 const unsigned int qp,
                                                 ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _u_dot(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosTimeDerivative<Derived>::computeQpJacobian(const unsigned int i,
                                                 const unsigned int j,
                                                 const unsigned int qp,
                                                 ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _phi(datum, j, qp) * _du_dot_du;
}

class KokkosTimeDerivativeKernel final : public KokkosTimeDerivative<KokkosTimeDerivativeKernel>
{
public:
  static InputParameters validParams();

  KokkosTimeDerivativeKernel(const InputParameters & parameters);
};

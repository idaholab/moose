//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernel.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos time-derivative kernels
 */
template <typename Derived>
class TimeKernel : public Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = Kernel<Derived>::validParams();

    params.set<MultiMooseEnum>("vector_tags") = "time";
    params.set<MultiMooseEnum>("matrix_tags") = "system time";

    return params;
  }

  /**
   * Constructor
   */
  TimeKernel(const InputParameters & parameters)
    : Kernel<Derived>(parameters),
      _u_dot(_var, Moose::SOLUTION_DOT_TAG),
      _du_dot_du(_var.sys().duDotDu(_var.number()))
  {
  }

  /**
   * Hook for additional computation for residual after the standard calls
   * @param ib The beginning element-local DOF index
   * @param ie The end element-local DOF index
   * @param datum The ResidualDatum object of the current thread
   * @param local_re The temporary storage storing the residual contribution of each DOF
   */
  KOKKOS_FUNCTION void computeResidualAdditional(const unsigned int /* ib */,
                                                 const unsigned int /* ie */,
                                                 ResidualDatum & /* datum */,
                                                 Real * /* local_re */) const
  {
  }

  /**
   * The parallel computation body that hides the base class method to allow additional computation
   * for residual through computeResidualAdditional()
   */
  KOKKOS_FUNCTION void computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const;

protected:
  /**
   * Time derivative of the current solution at quadrature points
   */
  VariableValue _u_dot;
  /**
   * Derivative of u_dot with respect to u
   */
  Scalar<const Real> _du_dot_du;
};

template <typename Derived>
KOKKOS_FUNCTION void
TimeKernel<Derived>::computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const
{
  Real local_re[MAX_DOF];

  unsigned int num_batches = datum.n_dofs() / MAX_DOF;

  if (datum.n_dofs() % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ib = batch * MAX_DOF;
    unsigned int ie = ::Kokkos::min(ib + MAX_DOF, datum.n_dofs());

    for (unsigned int i = ib; i < ie; ++i)
      local_re[i - ib] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int i = ib; i < ie; ++i)
        local_re[i - ib] += datum.JxW(qp) * kernel->computeQpResidual(i, qp, datum);
    }

    kernel->computeResidualAdditional(ib, ie, datum, local_re);

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosTimeKernelMembers(T)                                                            \
  usingKokkosKernelMembers(T);                                                                     \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::TimeKernel<T>::_u_dot;                                                      \
  using Moose::Kokkos::TimeKernel<T>::_du_dot_du

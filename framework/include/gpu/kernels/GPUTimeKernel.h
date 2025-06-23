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

template <typename Kernel>
class GPUTimeKernel : public GPUKernel<Kernel>
{
  usingGPUKernelMembers(Kernel);

public:
  static InputParameters validParams()
  {
    InputParameters params = GPUKernel<Kernel>::validParams();

    params.set<MultiMooseEnum>("vector_tags") = "time";
    params.set<MultiMooseEnum>("matrix_tags") = "system time";

    return params;
  }

  // Constructor
  GPUTimeKernel(const InputParameters & parameters)
    : GPUKernel<Kernel>(parameters),
      _u_dot(systems(), _var, Moose::SOLUTION_DOT_TAG),
      _du_dot_du(_var.sys().duDotDu(_var.number()))
  {
  }

  // Empty method to prevent compile errors even when this method was not hidden by the derived
  // class
  KOKKOS_FUNCTION void computeResidualAdditional(Real * /* local_re */,
                                                 ResidualDatum & /* datum */) const
  {
  }

  KOKKOS_FUNCTION void
  computeResidualInternal(const Kernel * kernel, ResidualDatum & datum, Real * local_re) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_dofs(); ++i)
        local_re[i] += datum.JxW(qp) * kernel->computeQpResidual(i, qp, datum);
    }

    kernel->computeResidualAdditional(local_re, datum);

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      accumulateTaggedLocalResidual(local_re[i], datum.elem().id, i);
  }

protected:
  /// Time derivative of u
  GPUVariableValue _u_dot;
  /// Derivative of u_dot with respect to u
  GPUScalar<const Real> _du_dot_du;
};

#define usingGPUTimeKernelMembers(T)                                                               \
  usingGPUKernelMembers(T);                                                                        \
                                                                                                   \
protected:                                                                                         \
  using GPUTimeKernel<T>::_u_dot;                                                                  \
  using GPUTimeKernel<T>::_du_dot_du;

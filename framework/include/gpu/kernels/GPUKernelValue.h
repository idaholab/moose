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
class GPUKernelValue : public GPUKernel<Kernel>
{
  usingGPUKernelMembers(Kernel);

public:
  static InputParameters validParams()
  {
    InputParameters params = GPUKernel<Kernel>::validParams();
    return params;
  }

  // Constructor
  GPUKernelValue(const InputParameters & parameters) : GPUKernel<Kernel>(parameters) {}

  // Empty method to prevent compile errors even when this method was not hidden by the derived
  // class
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            ResidualDatum & /* datum */) const
  {
    return 0;
  }

  KOKKOS_FUNCTION void
  computeResidualInternal(const Kernel * kernel, ResidualDatum & datum, Real * local_re) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      Real value = datum.JxW(qp) * kernel->precomputeQpResidual(qp, datum);

      for (unsigned int i = 0; i < datum.n_dofs(); ++i)
        local_re[i] += value * _test(datum, i, qp);
    }

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      accumulateTaggedLocalResidual(local_re[i], datum.elem().id, i);
  }
  KOKKOS_FUNCTION void
  computeJacobianInternal(const Kernel * kernel, ResidualDatum & datum, Real * local_ke) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
      {
        Real value = datum.JxW(qp) * kernel->precomputeQpJacobian(j, qp, datum);

        for (unsigned int i = 0; i < datum.n_idofs(); ++i)
          local_ke[i * datum.n_jdofs() + j] += value * _test(datum, i, qp);
      }
    }

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        accumulateTaggedLocalMatrix(
            local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
  }

protected:
  virtual bool defaultJacobian() const override
  {
    return &Kernel::precomputeQpJacobian == &GPUKernelValue::precomputeQpJacobian;
  }
};

#define usingGPUKernelValueMembers(T) usingGPUKernelMembers(T)

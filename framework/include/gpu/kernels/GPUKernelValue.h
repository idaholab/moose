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
 * The base class for a user to derive his own Kokkos kernels where the residual is of the form
 *
 * $(\dots, \psi_i)$
 *
 * i.e. the test function $(\psi_i)$ can be factored out for optimization.
 *
 * The user is expected to define precomputeQpResidual() and precomputeQpJacobian() instead of
 * computeQpResidual() and computeQpJacobian(). The signature of precomputeQpResidual() expected to
 * be defined in the derived class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The ResidualDatum object of the current thread
 * @returns The component of the residual contribution that will be multiplied by the test function
 *
 * KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp,
 *                                           ResidualDatum & datum) const;
 *
 * The signature of precomputeQpJacobian() can be found in the code below. The definition of
 * computeQpOffDiagJacobian() is still the same with the original Kokkos kernel.
 */
template <typename Derived>
class KernelValue : public Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = Kernel<Derived>::validParams();
    return params;
  }

  /**
   * Constructor
   */
  KernelValue(const InputParameters & parameters) : Kernel<Derived>(parameters) {}

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Compute diagonal Jacobian contribution on a quadrature point
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The ResidualDatum object of the current thread
   * @returns The component of the Jacobian contribution that will be multiplied by the test
   * function
   */
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            ResidualDatum & /* datum */) const
  {
    return 0;
  }
  ///@}

  /**
   * The parallel computation bodies that hide the base class methods to optimize for factoring
   * out the test function
   */
  ///@{
  KOKKOS_FUNCTION void
  computeResidualInternal(const Derived * kernel, ResidualDatum & datum, Real * local_re) const;
  KOKKOS_FUNCTION void
  computeJacobianInternal(const Derived * kernel, ResidualDatum & datum, Real * local_ke) const;
  ///@}

protected:
  /**
   * Get whether precomputeQpJacobian() was not defined in the derived class
   * @returns Whether precomputeQpJacobian() was not defined in the derived class
   */
  virtual bool defaultJacobian() const override
  {
    return &Derived::precomputeQpJacobian == &KernelValue::precomputeQpJacobian;
  }
};

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue<Derived>::computeResidualInternal(const Derived * kernel,
                                              ResidualDatum & datum,
                                              Real * local_re) const
{
  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit(qp);

    Real value = datum.JxW(qp) * kernel->precomputeQpResidual(qp, datum);

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      local_re[i] += value * _test(datum, i, qp);
  }

  for (unsigned int i = 0; i < datum.n_dofs(); ++i)
    accumulateTaggedElementalResidual(local_re[i], datum.elem().id, i);
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue<Derived>::computeJacobianInternal(const Derived * kernel,
                                              ResidualDatum & datum,
                                              Real * local_ke) const
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
      accumulateTaggedElementalMatrix(
          local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosKernelValueMembers(T) usingKokkosKernelMembers(T)

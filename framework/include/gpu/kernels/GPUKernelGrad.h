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
 * The base class for a user to derive their own Kokkos kernels where the residual is of the form
 *
 * $(\dots, \nabla\psi_i)$
 *
 * i.e. the gradient of the test function $(\nabla\psi_i)$ can be factored out for optimization.
 *
 * The user is expected to define precomputeQpResidual() and precomputeQpJacobian() instead of
 * computeQpResidual() and computeQpJacobian(). The signature of precomputeQpResidual() expected to
 * be defined in the derived class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The ResidualDatum object of the current thread
 * @returns The vector component of the residual contribution that will be multiplied by the
 * gradient of the test function
 *
 * KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp,
 *                                            ResidualDatum & datum) const;
 *
 * The signature of precomputeQpJacobian() can be found in the code below. The definition of
 * computeQpOffDiagJacobian() is still the same with the original Kokkos kernel.
 */
template <typename Derived>
class KernelGrad : public Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  KernelGrad(const InputParameters & parameters);

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
   * @returns The vector component of the Jacobian contribution that will be multiplied by the
   * gradient of the test function
   */
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int /* j */,
                                             const unsigned int /* qp */,
                                             ResidualDatum & /* datum */) const
  {
    return Real3(0);
  }
  ///@}

  /**
   * The parallel computation bodies that hide the base class methods to optimize for factoring
   * out the gradient of test function
   */
  ///@{
  KOKKOS_FUNCTION void computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const;
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const;
  ///@}

protected:
  /**
   * Get whether precomputeQpJacobian() was not defined in the derived class
   * @returns Whether precomputeQpJacobian() was not defined in the derived class
   */
  virtual bool defaultJacobian() const override
  {
    return &Derived::precomputeQpJacobian == &KernelGrad::precomputeQpJacobian;
  }
};

template <typename Derived>
InputParameters
KernelGrad<Derived>::validParams()
{
  InputParameters params = Kernel<Derived>::validParams();
  return params;
}

template <typename Derived>
KernelGrad<Derived>::KernelGrad(const InputParameters & parameters) : Kernel<Derived>(parameters)
{
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelGrad<Derived>::computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          Real3 value = datum.JxW(qp) * kernel->precomputeQpResidual(qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _grad_test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelGrad<Derived>::computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const
{
  Real3 value;

  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ijb, const unsigned int ije)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          unsigned int j_old = libMesh::invalid_uint;

          for (unsigned int ij = ijb; ij < ije; ++ij)
          {
            unsigned int i = ij % datum.n_jdofs();
            unsigned int j = ij / datum.n_jdofs();

            if (j != j_old)
            {
              value = datum.JxW(qp) * kernel->precomputeQpJacobian(j, qp, datum);
              j_old = j;
            }

            local_ke[ij] += value * _grad_test(datum, i, qp);
          }
        }
      });
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosKernelGradMembers(T) usingKokkosKernelMembers(T)

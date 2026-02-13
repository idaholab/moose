//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels where the residual is of the form
 *
 * $(\dots, \psi_i)$
 *
 * i.e. the test function $(\psi_i)$ can be factored out for optimization.
 *
 * The user should still define computeQpResidual() and computeQpJacobian(), but their signatures
 * are different from the base class. The signature of computeQpResidual() expected to be defined in
 * the derived class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The component of the residual contribution that will be multiplied by the test function
 *
 * KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp,
 *                                        AssemblyDatum & datum) const;
 *
 * The signature of computeQpJacobian() can be found in the code below. The definition of
 * computeQpOffDiagJacobian() is still the same with the original Kokkos kernel.
 */
class KernelValue : public Kernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  KernelValue(const InputParameters & parameters);

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Compute diagonal Jacobian contribution on a quadrature point
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The component of the Jacobian contribution that will be multiplied by the test
   * function
   */
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Get the function pointer of the default computeQpJacobian()
   * @returns The function pointer
   */
  static auto defaultJacobian() { return &KernelValue::computeQpJacobian; }
  ///@}

  /**
   * Shims for hook methods that can be leveraged to implement static polymorphism
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidualShim(const Derived & kernel,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
  {
    return kernel.computeQpResidual(qp, datum);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobianShim(const Derived & kernel,
                                             const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
  {
    return kernel.computeQpJacobian(j, qp, datum);
  }
  ///@}

  /**
   * The parallel computation bodies that hide the base class methods to optimize for factoring
   * out the test function
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          Real value = datum.JxW(qp) * kernel.computeQpResidualShim(kernel, qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  Real value = 0;

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
              value = datum.JxW(qp) * kernel.computeQpJacobianShim(kernel, j, qp, datum);
              j_old = j;
            }

            local_ke[ij] += value * _test(datum, i, qp);
          }
        }
      });
}

} // namespace Moose::Kokkos

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
 * The user should still define computeQpResidual(), computeQpJacobian(), and
 * computeQpOffDiagJacobian(), but their signatures are different from the base class. The signature
 * of computeQpResidual() expected to be defined in the derived class is as follows:
 *
 * @tparam Derived The object type
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The component of the residual contribution that will be multiplied by the test function
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp,
 *                                        AssemblyDatum & datum) const;
 *
 * The signature of computeQpJacobian() and computeQpOffDiagJacobian() can be found in the code
 * below.
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
   * @tparam Derived The object type
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The component of the Jacobian contribution that will be multiplied by the test
   * function
   */
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort("Default computeQpJacobian() should never be called. Make sure you properly "
                    "redefined this method in your class without typos.");

    return 0;
  }
  /**
   * Compute off-diagonal Jacobian contribution on a quadrature point
   * @tparam Derived The object type
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The component of the off-diagonal Jacobian contribution that will be multiplied by the
   * test function
   */
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* j */,
                                                const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort(
        "Default computeQpOffDiagJacobian() should never be called. Make sure you properly "
        "redefined this method in your class without typos.");

    return 0;
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   * @returns The function pointer of the default hook method
   */
  ///@{
  template <typename Derived>
  static auto defaultJacobian()
  {
    return &KernelValue::computeQpJacobian<Derived>;
  }
  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &KernelValue::computeQpOffDiagJacobian<Derived>;
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
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & kernel,
                                                      AssemblyDatum & datum) const;
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
          Real value = datum.JxW(qp) * kernel.template computeQpResidual<Derived>(qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real value = datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(j, qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue::computeOffDiagJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real value = datum.JxW(qp) * kernel.template computeQpOffDiagJacobian<Derived>(
                                           j, datum.jvar(), qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

} // namespace Moose::Kokkos

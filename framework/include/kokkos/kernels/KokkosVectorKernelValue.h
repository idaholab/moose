//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernel.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos vector kernels where the residual is of the form
 *
 * $(\dots, \psi_i)$
 *
 * i.e. the vector test function $(\psi_i)$ can be factored out for optimization.
 */
class VectorKernelValue : public VectorKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  VectorKernelValue(const InputParameters & parameters);

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real3 computeQpJacobian(const unsigned int /* j */,
                                          const unsigned int /* qp */,
                                          AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort("Default computeQpJacobian() should never be called. Make sure you properly "
                    "redefined this method in your class without typos.");

    return Real3(0);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real3 computeQpOffDiagJacobian(const unsigned int /* j */,
                                                 const unsigned int /* jvar */,
                                                 const unsigned int /* qp */,
                                                 AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort(
        "Default computeQpOffDiagJacobian() should never be called. Make sure you properly "
        "redefined this method in your class without typos.");

    return Real3(0);
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   */
  ///@{
  template <typename Derived>
  static auto defaultJacobian()
  {
    return &VectorKernelValue::computeQpJacobian<Derived>;
  }
  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &VectorKernelValue::computeQpOffDiagJacobian<Derived>;
  }
  ///@}

  /**
   * Optimized computation bodies that factor out the vector test function
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
VectorKernelValue::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * kernel.template computeQpResidual<Derived>(qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernelValue::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(j, qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernelValue::computeOffDiagJacobianInternal(const Derived & kernel,
                                                  AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * kernel.template computeQpOffDiagJacobian<Derived>(
                                            j, datum.jvar(), qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

} // namespace Moose::Kokkos

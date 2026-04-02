//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels using automatic differentiation
 * (AD).
 *
 * The user should define computeQpResidual() as inlined public method in their derived class (not
 * virtual override). The signature of computeQpResidual() expected to be defined in the derived
 * class is as follows:
 *
 * @tparam Derived The object type
 * @param i The element-local DOF index
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The residual contribution
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Moose::Kokkos::ADReal computeQpResidual(const unsigned int i,
 *                                                         const unsigned int qp,
 *                                                         AssemblyDatum & datum) const;
 *
 * Note that computeQpJacobian() and computeQpOffDiagJacobian() are unused for AD kernels.
 */
class ADKernel : public KernelBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  ADKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override { computeResidual(); }

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const;

  /**
   * Compute residual
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;

protected:
  /**
   * Current test function
   */
  const ADVariableTestValue _test;
  /**
   * Gradient of the current test function
   */
  const ADVariableTestGradient _grad_test;
  /**
   * Current shape function
   */
  const ADVariablePhiValue _phi;
  /**
   * Gradient of the current shape function
   */
  const ADVariablePhiGradient _grad_phi;
  /**
   * Current solution at quadrature points
   */
  const ADVariableValue _u;
  /**
   * Gradient of the current solution at quadrature points
   */
  const ADVariableGradient _grad_u;
};

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(tid);

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var());

  kernel.computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeADResidualInternal(
      datum,
      [&](ADReal & local_re, const unsigned int i)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          local_re += datum.JxW(qp) * kernel.template computeQpResidual<Derived>(i, qp, datum);
        }
      });
}

} // namespace Moose::Kokkos

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
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

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
   * Dispatch parallel calculation
   */
  virtual void dispatch();

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
  /**
   * Whether computing residual
   */
  bool _computing_residual = false;
  /**
   * Whether computing Jacobian
   */
  bool _computing_jacobian = false;
};

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var());

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  datum.do_derivatives(_computing_jacobian);

  kernel.computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  for (unsigned int i = datum.local_thread_id(); i < datum.n_dofs(); i += datum.num_local_threads())
  {
    ADReal local_re = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
      local_re += datum.JxW(qp) * kernel.template computeQpResidual<Derived>(i, qp, datum);

    if (_computing_residual)
      accumulateTaggedElementalResidual(local_re.value(), datum.elem().id, i);
    if (_computing_jacobian)
      accumulateTaggedElementalMatrix(local_re.derivatives(), datum, i);
  }
}

} // namespace Moose::Kokkos

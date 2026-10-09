//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelBase.h"
#include "KokkosPrecomputeHooks.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels on vector variables.
 *
 * A Kokkos vector kernel computes its residual contribution as a sum of terms, each the product of
 * a coefficient computed once per quadrature point and one test function quantity:
 *
 *   R_i = sum_qp JxW (f_value . psi_i + f_grad : grad psi_i + f_curl . curl psi_i)
 *
 * The coefficient hooks are precomputeQpResidual{Value,Grad,Curl}(qp, datum), returning Real3,
 * Real33, and Real3, respectively, of which at least one must be defined, and the optional
 * Jacobian hooks precomputeQpJacobian{Value,Grad,Curl}(j, qp, datum) and
 * precomputeQpOffDiagJacobian{Value,Grad,Curl}(j, jvar, qp, datum). Only the terms whose hooks
 * are defined are assembled. KokkosPrecomputeHookTraits.h documents the hook families.
 */
class VectorKernel : public KernelBase
{
public:
  static InputParameters validParams();

  /// Vector kernels assemble the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = true;

  /**
   * Constructor
   */
  VectorKernel(const InputParameters & parameters);

  /**
   * Dispatch residual calculation
   */
  virtual void computeResidual() override;
  /**
   * Dispatch diagonal and off-diagonal Jacobian calculation
   */
  virtual void computeJacobian() override;

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const;
  ///@}

  /**
   * The parallel computation bodies, which a derived class may hide by defining methods with the
   * same signature as inlined public methods
   */
  ///@{
  /**
   * Compute residual
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & kernel,
                                                      AssemblyDatum & datum) const;
  ///@}

protected:
  /**
   * Current vector test function
   */
  const VectorVariableTestValue _test;
  /**
   * Gradient of the current vector test function
   */
  const VectorVariableTestGradient _grad_test;
  /**
   * Current vector shape function
   */
  const VectorVariablePhiValue _phi;
  /**
   * Gradient of the current vector shape function
   */
  const VectorVariablePhiGradient _grad_phi;
  /**
   * Current vector solution at quadrature points
   */
  const VectorVariableValue _u;
  /**
   * Gradient of the current vector solution at quadrature points
   */
  const VectorVariableGradient _grad_u;
  /**
   * Curl of the current vector test function
   */
  const VectorVariableTestCurl _curl_test;
  /**
   * Curl of the current vector shape function
   */
  const VectorVariablePhiCurl _curl_phi;
  /**
   * Curl of the current vector solution at quadrature points
   */
  const VectorVariableCurl _curl_u;
};

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var());

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var());

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 2));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getFieldCoupling(_kokkos_var.var())[_thread(tid, 1)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeOffDiagJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(datum, precomputeResidualBody<true>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeJacobianBody<true>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::computeOffDiagJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum,
                                          precomputeOffDiagJacobianBody<true>(kernel, datum));
}

} // namespace Moose::Kokkos

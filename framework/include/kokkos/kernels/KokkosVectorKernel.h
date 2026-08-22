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
 * The base class for a user to derive their own Kokkos vector kernels on vector variables.
 */
class VectorKernel : public KernelBase
{
public:
  static InputParameters validParams();

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
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Compute diagonal Jacobian contribution on a quadrature point
   * @tparam Derived The object type
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The diagonal Jacobian contribution
   */
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* i */,
                                         const unsigned int /* j */,
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
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The off-diagonal Jacobian contribution
   */
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
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
    return &VectorKernel::computeQpJacobian<Derived>;
  }
  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &VectorKernel::computeQpOffDiagJacobian<Derived>;
  }
  ///@}

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
   * The parallel computation bodies that can be customized in the derived class by defining
   * them in the derived class with the same signature.
   * Make sure to define them as inlined public methods if to be defined in the derived class.
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
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 1)];

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
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += datum.JxW(qp) * kernel.template computeQpResidual<Derived>(i, qp, datum);
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] +=
                datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(i, j, qp, datum);
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorKernel::computeOffDiagJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += datum.JxW(qp) * kernel.template computeQpOffDiagJacobian<Derived>(
                                               i, j, datum.jvar(), qp, datum);
      });
}

} // namespace Moose::Kokkos

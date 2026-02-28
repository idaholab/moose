//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalKernelBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos nodal kernels.
 *
 * The user should define computeQpResidual(), computeQpJacobian(), and computeQpOffDiagJacobian()
 * as inlined public methods in their derived class (not virtual override). The signature of
 * computeQpResidual() expected to be defined in the derived class is as follows:
 *
 * @param qp The dummy quadrature point index (= 0)
 * @param datum The AssemblyDatum object of the current thread
 * @returns The residual contribution
 *
 * KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
 *
 * The signatures of computeQpJacobian() and computeOffDiagQpJacobian() can be found in the code
 * below, and their definition in the derived class is optional. If they are defined in the derived
 * class, they will hide the default definitions in the base class.
 */
class NodalKernel : public NodalKernelBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  NodalKernel(const InputParameters & parameters);

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
   * Compute diagonal Jacobian contribution on a node
   * @param qp The dummy quadrature point index (= 0)
   * @param datum The AssemblyDatum object of the current thread
   * @returns The diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Compute off-diagonal Jacobian contribution on a node
   * @param jvar The variable number for column
   * @param qp The dummy quadrature point index (= 0)
   * @param datum The AssemblyDatum object of the current thread
   * @returns The off-diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Get the function pointer of the default computeQpJacobian()
   * @returns The function pointer
   */
  static auto defaultJacobian() { return &NodalKernel::computeQpJacobian; }
  /**
   * Get the function pointer of the default computeQpOffDiagJacobian()
   * @returns The function pointer
   */
  static auto defaultOffDiagJacobian() { return &NodalKernel::computeQpOffDiagJacobian; }
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
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
  {
    return kernel.computeQpJacobian(qp, datum);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobianShim(const Derived & kernel,
                                                    const unsigned int jvar,
                                                    const unsigned int qp,
                                                    AssemblyDatum & datum) const
  {
    return kernel.computeQpOffDiagJacobian(jvar, qp, datum);
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

protected:
  /**
   * Current solution at nodes
   */
  const VariableValue _u;

private:
  /**
   * Flag whether this kernel is boundary-restricted
   */
  const bool _boundary_restricted;
};

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  auto comp = _thread(tid, 0);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 1))
                                   : kokkosBlockNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));

  if (!sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  AssemblyDatum datum(
      node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var(comp), comp);

  Real local_re = kernel.computeQpResidualShim(kernel, 0, datum);

  accumulateTaggedNodalResidual(true, local_re, node, comp);
}

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel::operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto comp = _thread(tid, 0);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 1))
                                   : kokkosBlockNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));

  if (!sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  AssemblyDatum datum(
      node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var(comp), comp);

  Real local_ke = kernel.computeQpJacobianShim(kernel, 0, datum);

  accumulateTaggedNodalMatrix(true, local_ke, node, _kokkos_var.var(comp), comp);
}

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto comp = _thread(tid, 0);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 2))
                                   : kokkosBlockNodeID(_thread(tid, 2));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto jvar = sys.getCoupling(_kokkos_var.var(comp))[_thread(tid, 1)];

  if (!sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, comp);

  Real local_ke = kernel.computeQpOffDiagJacobianShim(kernel, jvar, 0, datum);

  accumulateTaggedNodalMatrix(true, local_ke, node, jvar, comp);
}

} // namespace Moose::Kokkos

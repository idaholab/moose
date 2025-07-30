//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalKernelBase.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive his own Kokkos nodal kernels.
 *
 * The polymorphic design of the original MOOSE is reproduced statically by leveraging the Curiously
 * Recurring Template Pattern (CRTP), a programming idiom that involves a class template inheriting
 * from a template instantiation of itself. When the user derives his Kokkos object from this class,
 * the inheritance structure will look like:
 *
 * class UserNodalKernel final : public Moose::Kokkos::NodalKernel<UserNodalKernel>
 *
 * It is important to note that the template argument should point to the last derived class.
 * Therefore, if the user wants to define a derived class that can be further inherited, the derived
 * class should be a class template as well. Otherwise, it is recommended to mark the derived class
 * as final to prevent its inheritence by mistake.
 *
 * The user is expected to define computeQpResidual(), computeQpJacobian(), and
 * computeQpOffDiagJacobian() as inlined public methods in his derived class (not virtual override).
 * The signature of computeQpResidual() expected to be defined in the derived class is as follows:
 *
 * @param node The node ID
 * @returns The residual contribution
 *
 * KOKKOS_FUNCTION Real computeQpResidual(const dof_id_type node) const;
 *
 * The signatures of computeQpJacobian() and computeOffDiagQpJacobian() can be found in the code
 * below, and their definition in the derived class is optional. If they are defined in the derived
 * class, they will hide the default definitions in the base class.
 */
template <typename Derived>
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
   * @param node The node ID
   * @returns The diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpJacobian(const dof_id_type /* node */) const { return 0; }
  /**
   * Compute off-diagonal Jacobian contribution on a node
   * @param jvar The variable number for column
   * @param node The node ID
   * @returns The off-diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
                                                const dof_id_type /* node */) const
  {
    return 0;
  }
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  KOKKOS_FUNCTION void operator()(ResidualLoop, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(JacobianLoop, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const dof_id_type tid) const;
  ///@}

protected:
  /**
   * Current solution at nodes
   */
  const VariableNodalValue _u;

private:
  /**
   * Flag whether this kernel is boundary-restricted
   */
  const bool _boundary_restricted;
  /**
   * Flag whether computeJacobian() was not defined in the derived class
   */
  const bool _default_diag;
  /**
   * Flag whether computeQpOffDiagJacobian() was not defined in the derived class
   */
  const bool _default_offdiag;
};

template <typename Derived>
InputParameters
NodalKernel<Derived>::validParams()
{
  InputParameters params = NodalKernelBase::validParams();
  return params;
}

template <typename Derived>
NodalKernel<Derived>::NodalKernel(const InputParameters & parameters)
  : NodalKernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(kokkosSystems(), _var),
    _boundary_restricted(boundaryRestricted()),
    _default_diag(&Derived::computeQpJacobian == &NodalKernel::computeQpJacobian),
    _default_offdiag(&Derived::computeQpOffDiagJacobian == &NodalKernel::computeQpOffDiagJacobian)
{
}

template <typename Derived>
void
NodalKernel<Derived>::computeResidual()
{
  ::Kokkos::RangePolicy<ResidualLoop, ExecSpace, ::Kokkos::IndexType<dof_id_type>> policy(
      0, _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes());
  ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
  ::Kokkos::fence();
}

template <typename Derived>
void
NodalKernel<Derived>::computeJacobian()
{
  if (!_default_diag)
  {
    ::Kokkos::RangePolicy<JacobianLoop, ExecSpace, ::Kokkos::IndexType<dof_id_type>> policy(
        0, _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }

  if (!_default_offdiag)
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize({sys.getCoupling(_kokkos_var.var()).size(),
                    _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes()});

    ::Kokkos::RangePolicy<OffDiagJacobianLoop, ExecSpace, ::Kokkos::IndexType<dof_id_type>> policy(
        0, _thread.size());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel<Derived>::operator()(ResidualLoop, const dof_id_type tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  Real local_re = kernel->computeQpResidual(node);

  accumulateTaggedNodalResidual(true, local_re, node);
}

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel<Derived>::operator()(JacobianLoop, const dof_id_type tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  Real local_ke = kernel->computeQpJacobian(node);

  accumulateTaggedNodalMatrix(true, local_ke, node, _kokkos_var.var());
}

template <typename Derived>
KOKKOS_FUNCTION void
NodalKernel<Derived>::operator()(OffDiagJacobianLoop, const dof_id_type tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 1))
                                   : kokkosBlockNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  Real local_ke = kernel->computeQpOffDiagJacobian(jvar, node);

  accumulateTaggedNodalMatrix(true, local_ke, node, jvar);
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosNodalKernelMembers(T)                                                           \
  usingKokkosNodalKernelBaseMembers;                                                               \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::NodalKernel<T>::_u;                                                         \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::NodalKernel<T>::operator()

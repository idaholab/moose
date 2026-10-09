//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCBase.h"
#include "KokkosPrecomputeHooks.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos integrated boundary conditions on vector
 * variables.
 *
 * The residual is assembled from the coefficient hooks precomputeQpResidual{Value,Grad,Curl}(qp,
 * datum), returning Real3, Real33, and Real3, respectively, of which at least one must be defined,
 * with the optional Jacobian hooks precomputeQpJacobian{Value,Grad,Curl}(j, qp, datum) and
 * precomputeQpOffDiagJacobian{Value,Grad,Curl}(j, jvar, qp, datum). Only the terms whose hooks
 * are defined are assembled. KokkosPrecomputeHookTraits.h documents the hook families.
 */
class VectorIntegratedBC : public IntegratedBCBase
{
public:
  static InputParameters validParams();

  /// Vector integrated BCs assemble the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = true;

  /**
   * Constructor
   */
  VectorIntegratedBC(const InputParameters & parameters);

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
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const;
  ///@}

  /**
   * The parallel computation bodies that can be customized in the derived class by defining
   * them in the derived class with the same signature.
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & bc,
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
VectorIntegratedBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeResidualInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBC::operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBC::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 2));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getFieldCoupling(_kokkos_var.var())[_thread(tid, 1)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeOffDiagJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBC::computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(datum, precomputeResidualBody<true>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBC::computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeJacobianBody<true>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBC::computeOffDiagJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeOffDiagJacobianBody<true>(bc, datum));
}

} // namespace Moose::Kokkos

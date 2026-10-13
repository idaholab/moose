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
 * The base class for a user to derive their own Kokkos integrated boundary conditions.
 *
 * A Kokkos integrated BC computes its residual contribution on a boundary side as a sum of terms,
 * each the product of a coefficient computed once per quadrature point and one test function
 * quantity:
 *
 *   R_i = sum_qp JxW (f_value psi_i + f_grad . grad psi_i)
 *
 * The coefficient hooks are precomputeQpResidual{Value,Grad}(qp, datum), returning Real and Real3,
 * respectively, of which at least one must be defined, and the optional Jacobian hooks
 * precomputeQpJacobian{Value,Grad}(j, qp, datum),
 * precomputeQpOffDiagJacobian{Value,Grad}(j, jvar, qp, datum), and
 * precomputeQpOffDiagJacobianScalar{Value,Grad}(j, jvar, qp, datum). Only the terms whose hooks
 * are defined are assembled. KokkosPrecomputeHookTraits.h documents the hook families.
 */
class IntegratedBC : public IntegratedBCBase
{
public:
  static InputParameters validParams();

  /// Integrated BCs assemble the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = true;
  /// Integrated BCs support the precomputeQpOffDiagJacobianScalar{Value,Grad}() hooks
  static constexpr bool supports_scalar_jacobian = true;

  /**
   * Constructor
   */
  IntegratedBC(const InputParameters & parameters);

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
  /**
   * The team operation called by Kokkos for each boundary side, assembling the Jacobian with
   * respect to a displacement component of an object posed on the current configuration
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DisplacementJacobianLoop,
                                  const TeamMember<DisplacementJacobianLoop> & team,
                                  const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianScalarLoop, const ThreadID tid, const Derived & bc) const;
  ///@}

  /**
   * The parallel computation bodies, which a derived class may hide by defining methods with the
   * same signature as inlined public methods
   */
  ///@{
  /**
   * Compute residual
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & bc,
                                                      AssemblyDatum & datum) const;
  /**
   * Compute scalar off-diagonal Jacobian
   * @param bc The integrated BC object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianScalarInternal(const Derived & bc,
                                                            AssemblyDatum & datum) const;
  ///@}

protected:
  /**
   * Current test function
   */
  const VariableTestValue _test;
  /**
   * Gradient of the current test function
   */
  const VariableTestGradient _grad_test;
  /**
   * Current shape function
   */
  const VariablePhiValue _phi;
  /**
   * Gradient of the current shape function
   */
  const VariablePhiGradient _grad_phi;
  /**
   * Current solution at quadrature points
   */
  const VariableValue _u;
  /**
   * Gradient of the current solution at quadrature points
   */
  const VariableGradient _grad_u;
};

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeResidualInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 2));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getFieldCoupling(_kokkos_var.var())[_thread(tid, 1)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  bc.computeOffDiagJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(OffDiagJacobianScalarLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getScalarCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, _configuration);

  bc.computeOffDiagJacobianScalarInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(datum, precomputeResidualBody<false>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeJacobianBody<false>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeOffDiagJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeOffDiagJacobianBody<false>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeOffDiagJacobianScalarInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum,
                                          precomputeOffDiagJacobianScalarBody<false>(bc, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(DisplacementJacobianLoop,
                         const TeamMember<DisplacementJacobianLoop> & team,
                         const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(team.league_rank());

  const auto & disp = _configuration.displacements();
  const auto c = _displacement_component;
  const auto jvar = disp.var(c);

  if (!kokkosSystem(_kokkos_var.sys())
           .isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, _configuration);

  assembleDisplacementJacobian<false>(
      bc,
      team,
      datum,
      c,
      [&](const Real local_ke, const unsigned int i, const unsigned int j)
      { accumulateTaggedElementalMatrix(local_ke, datum.elem().id, i, j, jvar); });
}

} // namespace Moose::Kokkos

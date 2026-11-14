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
   * The team operations called by Kokkos for each team of sides
   */
  ///@{
  template <typename Derived, typename Loop, bool displaced>
  KOKKOS_FUNCTION void operator()(DeformationLoop<Loop, displaced>,
                                  const TeamMember<DeformationLoop<Loop, displaced>> & team,
                                  const Derived & bc) const;
  /**
   * The team operation called by Kokkos for each boundary side, assembling the Jacobian with
   * respect to a displacement component of an object posed on the current configuration
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DisplacementJacobianLoop,
                                  const TeamMember<DisplacementJacobianLoop> & team,
                                  const Derived & bc) const;
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

template <typename Derived, typename Loop, bool displaced>
KOKKOS_FUNCTION void
IntegratedBC::operator()(DeformationLoop<Loop, displaced>,
                        const TeamMember<DeformationLoop<Loop, displaced>> & team,
                        const Derived & bc) const
{
  precomputeTeamLoop<Loop, displaced>(
      bc,
      team,
      numKokkosBoundarySides(),
      [&](const ThreadID e) { return kokkosBoundaryElementSideID(e); });
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(DisplacementJacobianLoop,
                         const TeamMember<DisplacementJacobianLoop> & team,
                         const Derived & bc) const
{
  const TeamGroup group(team,
                        _displacement_layout.max_qps,
                        DisplacementScratchLayout::align_groups,
                        numKokkosBoundarySides());
  const auto [elem, side] = kokkosBoundaryElementSideID(group.item());

  const auto & disp = _deformation.displacements();
  const auto c = _displacement_component;
  const auto jvar = disp.var(c);

  // A group whose displacement component is inactive on its subdomain still reaches the barriers
  const bool active =
      group.active() && kokkosSystem(_kokkos_var.sys())
                            .isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain);

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      jvar,
                      _deformation,
                      std::true_type{});

  const auto scaling_factor =
      kokkosSystem(_kokkos_var.sys()).getVariableScalingFactor(_kokkos_var.var());

  assembleDisplacementJacobian<false>(
      bc,
      team,
      group,
      _displacement_layout,
      active,
      datum,
      c,
      [&](const Real local_ke, const unsigned int i, const unsigned int j)
      { accumulateTaggedElementalMatrix(local_ke * scaling_factor, datum.elem().id, i, j, jvar); });
}

} // namespace Moose::Kokkos

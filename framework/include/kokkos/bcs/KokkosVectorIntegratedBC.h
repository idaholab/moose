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
  /// Vector objects assemble vector test functions
  static constexpr bool uses_vector_test_functions = true;

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

template <typename Derived, typename Loop, bool displaced>
KOKKOS_FUNCTION void
VectorIntegratedBC::operator()(DeformationLoop<Loop, displaced>,
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
VectorIntegratedBC::operator()(DisplacementJacobianLoop,
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

  assembleDisplacementJacobian<true>(
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

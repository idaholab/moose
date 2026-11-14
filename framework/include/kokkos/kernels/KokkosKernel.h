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
 * The base class for a user to derive their own Kokkos kernels.
 *
 * A Kokkos kernel computes its residual contribution as a sum of terms, each the product of a
 * coefficient computed once per quadrature point and one test function quantity:
 *
 *   R_i = sum_qp JxW (f_value psi_i + f_grad . grad psi_i)
 *
 * The user defines the coefficient hooks as inlined public template methods in their derived class
 * (not virtual overrides). The residual hooks, of which at least one must be defined, are
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
 *                                                AssemblyDatum & datum) const;
 * template <typename Derived>
 * KOKKOS_FUNCTION Real3 precomputeQpResidualGrad(const unsigned int qp,
 *                                                AssemblyDatum & datum) const;
 *
 * The Jacobian hooks follow the same pattern with additional arguments, and each is optional:
 * precomputeQpJacobian{Value,Grad}(j, qp, datum),
 * precomputeQpOffDiagJacobian{Value,Grad}(j, jvar, qp, datum), and
 * precomputeQpOffDiagJacobianScalar{Value,Grad}(j, jvar, qp, datum). Only the terms whose hooks
 * are defined are assembled. KokkosPrecomputeHookTraits.h documents the hook families.
 */
class Kernel : public KernelBase
{
public:
  static InputParameters validParams();

  /// Kernels assemble the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = true;
  /// Kernels support the precomputeQpOffDiagJacobianScalar{Value,Grad}() hooks
  static constexpr bool supports_scalar_jacobian = true;

  /**
   * Constructor
   */
  Kernel(const InputParameters & parameters);

  /**
   * Dispatch residual calculation
   */
  virtual void computeResidual() override;
  /**
   * Dispatch diagonal and off-diagonal Jacobian calculation
   */
  virtual void computeJacobian() override;

  /**
   * The team operations called by Kokkos for each team of elements
   */
  ///@{
  template <typename Derived, typename Loop, bool displaced>
  KOKKOS_FUNCTION void operator()(DeformationLoop<Loop, displaced>,
                                  const TeamMember<DeformationLoop<Loop, displaced>> & team,
                                  const Derived & kernel) const;
  /**
   * The team operation called by Kokkos for each element, assembling the Jacobian with respect to a
   * displacement component of an object posed on the current configuration
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DisplacementJacobianLoop,
                                  const TeamMember<DisplacementJacobianLoop> & team,
                                  const Derived & kernel) const;
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
Kernel::operator()(DeformationLoop<Loop, displaced>,
                   const TeamMember<DeformationLoop<Loop, displaced>> & team,
                   const Derived & kernel) const
{
  precomputeTeamLoop<Loop, displaced>(
      kernel,
      team,
      numKokkosBlockElements(),
      [&](const ThreadID e)
      {
        // A reference to libMesh::invalid_uint is not valid in device code
        const unsigned int side = libMesh::invalid_uint;
        return ::Kokkos::pair<ContiguousElementID, unsigned int>(kokkosBlockElementID(e), side);
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(DisplacementJacobianLoop,
                   const TeamMember<DisplacementJacobianLoop> & team,
                   const Derived & kernel) const
{
  const TeamGroup group(team,
                        _displacement_layout.max_qps,
                        DisplacementScratchLayout::align_groups,
                        numKokkosBlockElements());
  const auto elem = kokkosBlockElementID(group.item());
  const auto side = libMesh::invalid_uint;

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
      kernel,
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

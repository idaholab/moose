//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMortarConstraint.h"

class KokkosMortarContactKinematicsUserObject;

/**
 * Enforces the frictionless contact conditions g_j >= 0, lambda_j >= 0, g_j lambda_j = 0 at each
 * secondary node j with the nonlinear complementarity function
 *
 *   R_j = min(lambda_j, c_j g_j),
 *
 * where g_j is the weighted gap of KokkosMortarContactKinematicsUserObject and c_j = c, or c
 * divided by the normalization of the weighted gap with normalize_c. On a node where the gap branch
 * is chosen, the row is assembled point by point over the mortar segments that reach the node,
 * since the weighted gap is a sum over those points, so each process adds the points of its local
 * secondary elements. Its Jacobian holds the derivatives of the gap vector with respect to the
 * displacements of both faces and of the nodal normal with respect to the positions of the
 * secondary face nodes. On a node where the multiplier branch is chosen, the owner of the node
 * assembles lambda_j.
 */
class KokkosComputeWeightedGapLMMechanicalContact : public Moose::Kokkos::MortarConstraintBase
{
public:
  static InputParameters validParams();

  KokkosComputeWeightedGapLMMechanicalContact(const InputParameters & parameters);
  KokkosComputeWeightedGapLMMechanicalContact(
      const KokkosComputeWeightedGapLMMechanicalContact & object);

  using MortarConstraintBase::operator();

  /// Kokkos function tag of the gap branch, one team per local secondary element
  struct MortarLoop
  {
  };

  /// Kokkos function tag of the multiplier branch, one thread per owned secondary node
  struct NodeLoop
  {
  };

  KOKKOS_FUNCTION void operator()(MortarLoop,
                                  const Moose::Kokkos::TeamMember<MortarLoop> & team,
                                  const KokkosComputeWeightedGapLMMechanicalContact &) const;

  KOKKOS_FUNCTION void operator()(NodeLoop,
                                  const Moose::Kokkos::ThreadID tid,
                                  const KokkosComputeWeightedGapLMMechanicalContact &) const;

protected:
  virtual void dispatchMortar(Moose::Kokkos::ThreadID n_secondary) override;

private:
  /// The coefficient c_j of the weighted gap of a node
  KOKKOS_FUNCTION Real coefficient(const Real normalization) const
  {
    return _normalize_c ? _c / normalization : _c;
  }

  /// The kinematics user object
  const KokkosMortarContactKinematicsUserObject & _kinematics_uo;

  /// The coefficient that balances the gap against the contact pressure
  const Real _c;
  /// Whether c is divided by the normalization of the weighted gap
  const bool _normalize_c;

  /// The weighted gap and its normalization of each nodal normal of the interface geometry
  Moose::Kokkos::Array<Real> _gap;
  Moose::Kokkos::Array<Real> _normalization;

  /// The weighted gap, its normalization, and the local Lagrange multiplier degree of freedom of
  /// each owned secondary node that a mortar segment reaches
  Moose::Kokkos::Array<Real> _owned_gap;
  Moose::Kokkos::Array<Real> _owned_normalization;
  Moose::Kokkos::Array<dof_id_type> _owned_dofs;
  /// The revision of the owned nodes that the degrees of freedom were packed for
  unsigned int _owned_revision = 0;

  /// The dispatcher of the multiplier branch
  std::unique_ptr<Moose::Kokkos::DispatcherBase> _node_dispatcher;
};

KOKKOS_FUNCTION inline void
KokkosComputeWeightedGapLMMechanicalContact::operator()(
    MortarLoop,
    const Moose::Kokkos::TeamMember<MortarLoop> & team,
    const KokkosComputeWeightedGapLMMechanicalContact &) const
{
  constexpr auto S = static_cast<unsigned int>(MortarSide::Secondary);
  constexpr auto P = static_cast<unsigned int>(MortarSide::Primary);
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);
  constexpr auto max_face_nodes = Moose::Kokkos::MortarInterfaceGeometry::max_face_nodes;

  const auto secondary = team.league_rank();
  const auto begin = _geometry.secondary_qp_begin[secondary];
  const auto end = _geometry.secondary_qp_begin[secondary + 1];
  const auto lower = _geometry.elem[L][begin];
  const auto n_lambda =
      kokkosAssembly().getNumDofs(kokkosMesh().getElementInfo(lower).type, _lm_fe);
  const auto & sys = kokkosSystem(_lm_var.sys());

  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_lm_family, _lm_order, secondary, n_lambda, dual_coeff);

  for (dof_id_type qp = begin; qp < end; ++qp)
    ::Kokkos::parallel_for(
        ::Kokkos::TeamThreadRange(team, n_lambda),
        [&](const unsigned int k)
        {
          // The shape functions of the multiplier follow the nodes of the element
          const auto normal = _geometry.elem_node_normal[max_face_nodes * secondary + k];
          const auto c = coefficient(_normalization[normal]);
          const auto lambda = sys.getVectorDofValue(
              sys.getElemLocalDofIndex(lower, k, _lm_var.var()), _lm_var.tag());
          if (!(c * _gap[normal] < lambda))
            return;

          const auto & n = _geometry.nodal_normal[normal];
          const auto & d = _geometry.gap_vector[qp];
          const auto w =
              c * _geometry.JxW[qp] *
              _geometry.lowerBasis(
                  _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp);

          if (_computing_residual)
            accumulateTaggedResidual(
                w * (d(0) * n(0) + d(1) * n(1) + d(2) * n(2)), _lm_var, lower, k);

          if (_computing_jacobian)
          {
            // The gap vector moves with the displacements of both faces
            for (unsigned int comp = 0; comp < _n_disp; ++comp)
              if (displacementCoupled(_lm_var.var(), comp))
              {
                accumulateDisplacementJacobian(-w * n(comp), _lm_var, lower, k, comp, S, qp);
                accumulateDisplacementJacobian(w * n(comp), _lm_var, lower, k, comp, P, qp);
              }

            // The nodal normal turns with the secondary face nodes
            accumulateNormalJacobian(w * d, _lm_var, lower, k, normal);
          }
        });
}

KOKKOS_FUNCTION inline void
KokkosComputeWeightedGapLMMechanicalContact::operator()(
    NodeLoop,
    const Moose::Kokkos::ThreadID tid,
    const KokkosComputeWeightedGapLMMechanicalContact &) const
{
  const auto & sys = kokkosSystem(_lm_var.sys());
  const auto dof = _owned_dofs[tid];
  const auto lambda = sys.getVectorDofValue(dof, _lm_var.tag());
  if (coefficient(_owned_normalization[tid]) * _owned_gap[tid] < lambda)
    return;

  if (_computing_residual)
    accumulateTaggedDofResidual(lambda, _lm_var, dof);
  if (_computing_jacobian)
    accumulateTaggedDofMatrix(1, _lm_var, dof, sys.localToGlobalDofIndex(dof));
}

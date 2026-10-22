//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosCartesianMortarConstraintBase.h"
#include "KokkosDenseADReal.h"

class KokkosWeightedGapUserObject;

/**
 * Enforces frictional contact with a Cartesian Lagrange multiplier lambda_j at each secondary node
 * j. With the nodal normal n_j, the normal pressure lambda_n = lambda_j . n_j, the tangential
 * traction lambda_t = (I - n_j n_j^T) lambda_j, the weighted gap g_j = A_j . n_j of the weighted
 * gap vector A_j, and the tangential weighted slip s_t = (I - n_j n_j^T) B_j of the weighted
 * displacement increment B_j of KokkosWeightedGapUserObject, the residual is
 *
 *   R_n = min(lambda_n, c g_j),
 *   R_t = max(r, |q_t|) lambda_t - r q_t,  q_t = lambda_t + c_t s_t,  r = mu max(0, lambda_n - c
 * g_j),
 *
 * the degree-two Hueber-Stadler-Wohlmuth friction residual, with R_t = lambda_t where lambda_n is
 * below epsilon. R_n is the row of the multiplier component closest to the normal and the
 * components of R_t in the tangent frame of the Cartesian axes fill the other rows. The process
 * designated for a node assembles its rows: the derivatives with respect to lambda_j and n_j on
 * the node and those with respect to A_j and B_j point by point over its mortar segments, which
 * every process holding segments of the node adds.
 */
class KokkosComputeFrictionalForceCartesianLMMechanicalContact
  : public KokkosCartesianMortarConstraintBase
{
public:
  static InputParameters validParams();

  KokkosComputeFrictionalForceCartesianLMMechanicalContact(const InputParameters & parameters);
  KokkosComputeFrictionalForceCartesianLMMechanicalContact(
      const KokkosComputeFrictionalForceCartesianLMMechanicalContact & object);

  using MortarConstraintBase::operator();

  /// Kokkos function tag of the nodal residual and its derivatives, one thread per nodal normal
  struct NodeLoop
  {
  };

  /// Kokkos function tag of the derivatives with respect to the weighted gap vector and the
  /// weighted slip, one team per local secondary element
  struct MortarLoop
  {
  };

  KOKKOS_FUNCTION void
  operator()(NodeLoop,
             const Moose::Kokkos::ThreadID normal,
             const KokkosComputeFrictionalForceCartesianLMMechanicalContact &) const;

  KOKKOS_FUNCTION void
  operator()(MortarLoop,
             const Moose::Kokkos::TeamMember<MortarLoop> & team,
             const KokkosComputeFrictionalForceCartesianLMMechanicalContact &) const;

  /// Number of derivative slots of the nodal residual: lambda_j, n_j, A_j, and B_j
  static constexpr unsigned int slots = 12;
  /// First slot of each input of the nodal residual
  static constexpr unsigned int lambda_slot = 0, normal_slot = 3, gap_slot = 6, slip_slot = 9;

  using ADReal = Moose::Kokkos::DenseADReal<slots>;

protected:
  virtual void dispatchMortar(Moose::Kokkos::ThreadID n_secondary) override;

private:
  /**
   * Evaluate the nodal residual and its derivatives
   * @param normal The index of the nodal normal of the node
   * @param residual The residual of each row, in the order of the multiplier components
   */
  KOKKOS_FUNCTION void nodalResidual(dof_id_type normal, ADReal (&residual)[3]) const;

  /// The weighted gap user object
  const KokkosWeightedGapUserObject & _weighted_gap_uo;

  /// The coefficients that balance the gap and the slip against the contact pressure
  const Real _c, _c_t;
  /// Whether c and c_t are divided by the normalization of the weighted gap
  const bool _normalize_c;
  /// The Coulomb friction coefficient
  const Real _mu;
  /// The normal pressure below which the tangential traction is set to zero
  const Real _epsilon;

  /// The nodal quantities of the weighted gap user object, indexed by nodal normal
  Moose::Kokkos::Array<Moose::Kokkos::Real3> _gap_vector, _slip;
  Moose::Kokkos::Array<Real> _normalization;
  Moose::Kokkos::Array<bool> _designated;

  /// The dispatcher of the nodal loop
  std::unique_ptr<Moose::Kokkos::DispatcherBase> _node_dispatcher;
};

KOKKOS_FUNCTION inline void
KokkosComputeFrictionalForceCartesianLMMechanicalContact::nodalResidual(const dof_id_type normal,
                                                                        ADReal (&residual)[3]) const
{
  using Moose::Kokkos::seededDenseADReal;
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);

  const auto secondary = _geometry.normal_elem[normal];
  const auto k = _geometry.normal_elem_node[normal];
  const auto lower = _geometry.elem[L][_geometry.secondary_qp_begin[secondary]];
  const auto & sys = kokkosSystem(_lm[0].sys());
  KOKKOS_ASSERT(_geometry.elem_node_normal[Moose::Kokkos::MortarInterfaceGeometry::max_face_nodes *
                                               secondary +
                                           k] == normal);

  ADReal lambda[3], n[3], A[3], B[3];
  for (unsigned int c = 0; c < 3; ++c)
  {
    const auto value =
        c < _dim
            ? sys.getVectorDofValue(sys.getElemLocalDofIndex(lower, k, _lm[c].var()), _lm[c].tag())
            : 0;
    lambda[c] = seededDenseADReal<slots>(value, lambda_slot + c, 1);
    n[c] = seededDenseADReal<slots>(_geometry.nodal_normal[normal](c), normal_slot + c, 1);
    A[c] = seededDenseADReal<slots>(_gap_vector[normal](c), gap_slot + c, 1);
    B[c] = seededDenseADReal<slots>(_slip[normal](c), slip_slot + c, 1);
  }

  const Real scale = _normalize_c ? 1 / _normalization[normal] : 1;
  const Real c = _c * scale, c_t = _c_t * scale;

  const ADReal lambda_n = lambda[0] * n[0] + lambda[1] * n[1] + lambda[2] * n[2];
  const ADReal gap = A[0] * n[0] + A[1] * n[1] + A[2] * n[2];
  const ADReal slip_n = B[0] * n[0] + B[1] * n[1] + B[2] * n[2];

  ADReal lambda_t[3], q_t[3];
  for (unsigned int d = 0; d < 3; ++d)
  {
    lambda_t[d] = lambda[d] - lambda_n * n[d];
    q_t[d] = lambda_t[d] + c_t * (B[d] - slip_n * n[d]);
  }

  const ADReal R_n = c * gap < lambda_n ? c * gap : lambda_n;

  ADReal R_t[3];
  if (lambda_n.value() < _epsilon)
    for (unsigned int d = 0; d < 3; ++d)
      R_t[d] = lambda_t[d];
  else
  {
    const ADReal augmented = lambda_n - c * gap;
    const ADReal radius = augmented.value() > 0 ? _mu * augmented : ADReal(0);
    const ADReal q_norm2 = q_t[0] * q_t[0] + q_t[1] * q_t[1] + q_t[2] * q_t[2];
    // The weight is the larger of the radius and |q_t|, the radius on a tie, so |q_t| enters only
    // where it is positive and differentiable
    const ADReal weight =
        !(radius.value() * radius.value() < q_norm2.value()) ? radius : ADReal(sqrt(q_norm2));
    for (unsigned int d = 0; d < 3; ++d)
      R_t[d] = weight.value() == 0 ? lambda_t[d] : weight * lambda_t[d] - radius * q_t[d];
  }

  // The normal row is the multiplier component closest to the normal, and the tangential traction
  // fills the other rows along the Cartesian axes. The tangential components along the normal row
  // are a linear combination of the others because lambda_t is orthogonal to the normal.
  unsigned int normal_row = 0;
  for (unsigned int d = 1; d < _dim; ++d)
    if (::Kokkos::fabs(_geometry.nodal_normal[normal](d)) >
        ::Kokkos::fabs(_geometry.nodal_normal[normal](normal_row)))
      normal_row = d;

  for (unsigned int d = 0; d < _dim; ++d)
    residual[d] = d == normal_row ? R_n : R_t[d];
}

KOKKOS_FUNCTION inline void
KokkosComputeFrictionalForceCartesianLMMechanicalContact::operator()(
    NodeLoop,
    const Moose::Kokkos::ThreadID normal,
    const KokkosComputeFrictionalForceCartesianLMMechanicalContact &) const
{
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);

  if (!_designated[normal])
    return;

  ADReal residual[3];
  nodalResidual(normal, residual);

  const auto secondary = _geometry.normal_elem[normal];
  const auto k = _geometry.normal_elem_node[normal];
  const auto lower = _geometry.elem[L][_geometry.secondary_qp_begin[secondary]];

  for (unsigned int row = 0; row < _dim; ++row)
  {
    const auto & R = residual[row];

    if (_computing_residual)
      accumulateTaggedResidual(R.value(), _lm[row], lower, k);

    if (_computing_jacobian)
    {
      for (unsigned int c = 0; c < _dim; ++c)
        accumulateTaggedMatrix(
            R.derivatives()[lambda_slot + c], _lm[row], lower, k, _lm[c], lower, k);

      Moose::Kokkos::Real3 dR_dn;
      for (unsigned int c = 0; c < 3; ++c)
        dR_dn(c) = R.derivatives()[normal_slot + c];
      accumulateNormalJacobian(dR_dn, _lm[row], lower, k, normal);
    }
  }
}

KOKKOS_FUNCTION inline void
KokkosComputeFrictionalForceCartesianLMMechanicalContact::operator()(
    MortarLoop,
    const Moose::Kokkos::TeamMember<MortarLoop> & team,
    const KokkosComputeFrictionalForceCartesianLMMechanicalContact &) const
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

  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_lm_family, _lm_order, secondary, n_lambda, dual_coeff);

  // The rows of a node are those of its multiplier on whichever process is designated for it; the
  // gap and slip derivatives of the points of this element are added by this process
  ::Kokkos::parallel_for(
      ::Kokkos::TeamThreadRange(team, n_lambda),
      [&](const unsigned int k)
      {
        const auto normal = _geometry.elem_node_normal[max_face_nodes * secondary + k];

        ADReal residual[3];
        nodalResidual(normal, residual);

        // The rows live on the designated process's copy of the node, which shares its DOFs
        for (dof_id_type qp = begin; qp < end; ++qp)
        {
          const auto w =
              _geometry.JxW[qp] *
              _geometry.lowerBasis(
                  _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp);

          for (unsigned int row = 0; row < _dim; ++row)
          {
            const auto & dR = residual[row].derivatives();
            for (unsigned int c = 0; c < _n_disp; ++c)
            {
              if (!displacementCoupled(_lm[row].var(), c))
                continue;

              // dA/du_s = -psi phi_s, dA/du_p = psi phi_p; dB/du_s = psi phi_s, dB/du_p = -psi
              // phi_p
              const auto dA = w * dR[gap_slot + c];
              const auto dB = w * dR[slip_slot + c];
              accumulateDisplacementJacobian(dB - dA, _lm[row], lower, k, c, S, qp);
              accumulateDisplacementJacobian(dA - dB, _lm[row], lower, k, c, P, qp);
            }
          }
        }
      });
}

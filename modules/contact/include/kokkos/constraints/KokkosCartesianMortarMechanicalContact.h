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

/**
 * Applies the contact traction t = sum_j psi_j lambda_j of a Cartesian Lagrange multiplier to every
 * displacement component of the secondary and primary faces of a mortar interface:
 *
 *   R_secondary,i = sum_qp JxW t_c phi_secondary,i,  R_primary,i = -sum_qp JxW t_c phi_primary,i.
 *
 * The constraint leaves the rows of the multiplier to another constraint.
 */
class KokkosCartesianMortarMechanicalContact : public KokkosCartesianMortarConstraintBase
{
public:
  static InputParameters validParams();

  KokkosCartesianMortarMechanicalContact(const InputParameters & parameters);

  using MortarConstraintBase::operator();

  /// Kokkos function tag of the force, one team per local secondary element
  struct MortarLoop
  {
  };

  KOKKOS_FUNCTION void operator()(MortarLoop,
                                  const Moose::Kokkos::TeamMember<MortarLoop> & team,
                                  const KokkosCartesianMortarMechanicalContact &) const;

protected:
  virtual void dispatchMortar(Moose::Kokkos::ThreadID n_secondary) override;
};

KOKKOS_FUNCTION inline void
KokkosCartesianMortarMechanicalContact::operator()(
    MortarLoop,
    const Moose::Kokkos::TeamMember<MortarLoop> & team,
    const KokkosCartesianMortarMechanicalContact &) const
{
  constexpr auto S = static_cast<unsigned int>(MortarSide::Secondary);
  constexpr auto P = static_cast<unsigned int>(MortarSide::Primary);
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);
  constexpr auto max_face_nodes = Moose::Kokkos::MortarInterfaceGeometry::max_face_nodes;

  const auto secondary = team.league_rank();
  const auto begin = _geometry.secondary_qp_begin[secondary];
  const auto end = _geometry.secondary_qp_begin[secondary + 1];
  const auto lower = _geometry.elem[L][begin];
  const auto & mesh = kokkosMesh();
  const auto n_lambda = kokkosAssembly().getNumDofs(mesh.getElementInfo(lower).type, _lm_fe);
  const auto & lm_sys = kokkosSystem(_lm[0].sys());

  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_lm_family, _lm_order, secondary, n_lambda, dual_coeff);

  Real lambda[3][max_face_nodes];
  for (unsigned int c = 0; c < _dim; ++c)
    for (unsigned int k = 0; k < n_lambda; ++k)
      lambda[c][k] = lm_sys.getVectorDofValue(lm_sys.getElemLocalDofIndex(lower, k, _lm[c].var()),
                                              _lm[c].tag());

  for (dof_id_type qp = begin; qp < end; ++qp)
  {
    Real psi[max_face_nodes];
    Moose::Kokkos::Real3 t;
    for (unsigned int k = 0; k < n_lambda; ++k)
    {
      psi[k] = _geometry.lowerBasis(
          _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp);
      for (unsigned int c = 0; c < _dim; ++c)
        t(c) += psi[k] * lambda[c][k];
    }

    for (const unsigned int face : {S, P})
    {
      const auto elem = _geometry.elem[face][qp];
      const auto & info = mesh.getElementInfo(elem);
      const auto n_disp_dofs = kokkosAssembly().getNumDofs(info.type, _disp_fe);
      const Real sign = face == S ? 1 : -1;

      // The threads of the team split the test functions of the face
      ::Kokkos::parallel_for(
          ::Kokkos::TeamThreadRange(team, n_disp_dofs),
          [&](const unsigned int i)
          {
            const auto w =
                sign * _geometry.JxW[qp] * _geometry.shape(_disp_family, _disp_order, face, i, qp);

            for (unsigned int c = 0; c < _dim; ++c)
            {
              const auto & var = _disp_var[c];
              // A face whose block the displacement does not span carries no force
              if (!kokkosSystem(var.sys()).isVariableActive(var.var(), info.subdomain))
                continue;

              if (_computing_residual)
                accumulateTaggedResidual(w * t(c), var, elem, i);

              if (_computing_jacobian)
                for (unsigned int k = 0; k < n_lambda; ++k)
                  accumulateTaggedMatrix(w * psi[k], var, elem, i, _lm[c], lower, k);
            }
          });
    }
  }
}

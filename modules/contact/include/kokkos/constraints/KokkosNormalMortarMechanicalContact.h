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

/**
 * Applies the normal contact force of a Lagrange multiplier to every displacement component of the
 * secondary and primary faces of a mortar interface. With the traction
 *
 *   t = sum_j psi_j lambda_j n_j
 *
 * at a mortar segment quadrature point, the residual of component c is
 *
 *   R_secondary,i = sum_qp JxW t_c phi_secondary,i,  R_primary,i = -sum_qp JxW t_c phi_primary,i,
 *
 * the transpose of the weighted gap, so the two faces stay in equilibrium. Here psi is the basis
 * of the Lagrange multiplier and n_j the nodal normal of secondary node j. The Jacobian holds the
 * derivatives with respect to the multiplier and, through the nodal normals, to the positions of
 * the secondary face nodes. The constraint leaves the rows of the multiplier to another
 * constraint.
 */
class KokkosNormalMortarMechanicalContact : public Moose::Kokkos::MortarConstraintBase
{
public:
  static InputParameters validParams();

  KokkosNormalMortarMechanicalContact(const InputParameters & parameters);
  KokkosNormalMortarMechanicalContact(const KokkosNormalMortarMechanicalContact & object);

  using MortarConstraintBase::operator();

  /// Kokkos function tag of the force, one team per local secondary element
  struct MortarLoop
  {
  };

  KOKKOS_FUNCTION void operator()(MortarLoop,
                                  const Moose::Kokkos::TeamMember<MortarLoop> & team,
                                  const KokkosNormalMortarMechanicalContact &) const;

protected:
  virtual void dispatchMortar(Moose::Kokkos::ThreadID n_secondary) override;

private:
  /// Whether each displacement component is coupled to the Lagrange multiplier
  bool _lm_coupled[3];
};

KOKKOS_FUNCTION inline void
KokkosNormalMortarMechanicalContact::operator()(MortarLoop,
                                                const Moose::Kokkos::TeamMember<MortarLoop> & team,
                                                const KokkosNormalMortarMechanicalContact &) const
{
  constexpr auto S = static_cast<unsigned int>(MortarSide::Secondary);
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);
  constexpr auto max_face_nodes = Moose::Kokkos::MortarInterfaceGeometry::max_face_nodes;

  const auto secondary = team.league_rank();
  const auto begin = _geometry.secondary_qp_begin[secondary];
  const auto end = _geometry.secondary_qp_begin[secondary + 1];
  const auto lower = _geometry.elem[L][begin];
  const auto & mesh = kokkosMesh();
  const auto n_lambda = kokkosAssembly().getNumDofs(mesh.getElementInfo(lower).type, _lm_fe);
  const auto & lm_sys = kokkosSystem(_lm_var.sys());

  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_lm_family, _lm_order, secondary, n_lambda, dual_coeff);

  // The multiplier and the nodal normal of each node of the secondary element
  Real lambda[max_face_nodes];
  dof_id_type normal[max_face_nodes];
  for (unsigned int k = 0; k < n_lambda; ++k)
  {
    lambda[k] = lm_sys.getVectorDofValue(lm_sys.getElemLocalDofIndex(lower, k, _lm_var.var()),
                                         _lm_var.tag());
    normal[k] = _geometry.elem_node_normal[max_face_nodes * secondary + k];
  }

  for (dof_id_type qp = begin; qp < end; ++qp)
  {
    Real psi[max_face_nodes];
    Moose::Kokkos::Real3 t;
    for (unsigned int k = 0; k < n_lambda; ++k)
    {
      psi[k] = _geometry.lowerBasis(
          _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp);
      const auto & n = _geometry.nodal_normal[normal[k]];
      for (unsigned int d = 0; d < 3; ++d)
        t(d) += psi[k] * lambda[k] * n(d);
    }

    for (const unsigned int face : {S, static_cast<unsigned int>(MortarSide::Primary)})
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

            for (unsigned int c = 0; c < _n_disp; ++c)
            {
              const auto & var = _disp_var[c];
              // A face whose block the displacement does not span carries no force
              if (!kokkosSystem(var.sys()).isVariableActive(var.var(), info.subdomain))
                continue;

              if (_computing_residual)
                accumulateTaggedResidual(w * t(c), var, elem, i);

              if (_computing_jacobian)
                for (unsigned int k = 0; k < n_lambda; ++k)
                {
                  const auto & n = _geometry.nodal_normal[normal[k]];
                  if (_lm_coupled[c])
                    accumulateTaggedMatrix(w * psi[k] * n(c), var, elem, i, _lm_var, lower, k);

                  // The nodal normal turns with the secondary face nodes
                  Moose::Kokkos::Real3 coef;
                  coef(c) = w * psi[k] * lambda[k];
                  accumulateNormalJacobian(coef, var, elem, i, normal[k]);
                }
            }
          });
    }
  }
}

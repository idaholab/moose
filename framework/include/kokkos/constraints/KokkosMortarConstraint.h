//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosResidualObject.h"
#include "KokkosMortarInterfaceData.h"
#include "MortarExecutorInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

#include "libmesh/fe_dual_coeffs.h"
#include "libmesh/kokkos_fe_evaluator.h"

namespace Moose::Kokkos
{

/**
 * The quantities of one mortar segment quadrature point that the residual hooks of a Kokkos
 * mortar constraint read
 */
struct MortarDatum
{
  /// Secondary variable value
  Real u_secondary;
  /// Primary variable value
  Real u_primary;
  /// Lagrange multiplier value
  Real lambda;
  /// Signed distance from the secondary to the primary face along the secondary normal
  Real gap;
};

/**
 * The common base of Kokkos mortar constraints: a residual object whose variable is a Lagrange
 * multiplier on the lower-dimensional secondary elements of a mortar interface. It reads the
 * geometry the interface shares among its Kokkos objects, assembles the Jacobian columns of the
 * displacements of an interface posed on the current configuration, and pins each Lagrange
 * multiplier degree of freedom that no mortar segment reaches to lambda = 0. A derived class
 * dispatches its own loops over the local secondary elements from dispatchMortar().
 */
class MortarConstraintBase : public ResidualObject,
                             public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  MortarConstraintBase(const InputParameters & parameters);
  MortarConstraintBase(const MortarConstraintBase & object);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /// The side whose test functions a residual coefficient multiplies
  using MortarSide = Moose::Kokkos::MortarSide;

  /// Kokkos function tag of the pinning of the inactive Lagrange multiplier degrees of freedom
  struct InactiveLoop
  {
  };

  /**
   * The operation called by Kokkos for each inactive Lagrange multiplier degree of freedom
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(InactiveLoop, const ThreadID tid, const Derived &) const;

  /// Maximum number of shape functions of a variable on a lower-dimensional element
  static constexpr unsigned int max_shapes = MortarInterfaceGeometry::max_shapes;

protected:
  /**
   * Dispatch the loops of the derived class, after the geometry and the members of this class are
   * current. It is called on every process, including those without local secondary elements.
   * @param n_secondary The number of local secondary elements
   */
  virtual void dispatchMortar(ThreadID n_secondary) = 0;

  /**
   * Accumulate the Jacobian of a row with respect to one displacement component at the nodes of
   * the secondary or primary face at a quadrature point, through the displacement interpolated at
   * the point: the entries coef * phi_j of the displacement shape functions phi_j of that face
   * @param coef The derivative of the row with respect to the displacement component at the point
   * @param row_var The variable of the row
   * @param row_elem The contiguous element ID of the row
   * @param i The element-local DOF index of the row
   * @param c The displacement component
   * @param face The face, MortarSide::Secondary or MortarSide::Primary
   * @param qp The quadrature point
   */
  KOKKOS_FUNCTION void accumulateDisplacementJacobian(Real coef,
                                                      const Variable & row_var,
                                                      ContiguousElementID row_elem,
                                                      unsigned int i,
                                                      unsigned int c,
                                                      unsigned int face,
                                                      dof_id_type qp) const;

  /**
   * Accumulate the Jacobian of a row with respect to the displacement components at the nodes
   * that the nodal normal of a secondary node depends on: the entries coef . (d n / d x_k) for
   * each such node k and component
   * @param coef The derivative of the row with respect to the nodal normal
   * @param row_var The variable of the row
   * @param row_elem The contiguous element ID of the row
   * @param i The element-local DOF index of the row
   * @param normal The index of the nodal normal
   */
  KOKKOS_FUNCTION void accumulateNormalJacobian(const Real3 & coef,
                                                const Variable & row_var,
                                                ContiguousElementID row_elem,
                                                unsigned int i,
                                                dof_id_type normal) const;

  /**
   * Get whether a row variable is coupled to a displacement component
   * @param row The variable number of the row
   * @param c The displacement component
   */
  KOKKOS_FUNCTION bool displacementCoupled(unsigned int row, unsigned int c) const;

  /// The host-side mortar interface, shared by the copies of the constraint
  std::shared_ptr<MortarConsumer> _interface;

  /// The host-side mortar interface, which the device copies of the constraint do not carry
  const AutomaticMortarGeneration & amg() const { return _interface->amg(); }

  /// The geometry of the interface, shared by the Kokkos objects posed on it
  MortarInterfaceData & _interface_data;

  /// The geometry of the interface as of the current dispatch
  MortarInterfaceGeometry _geometry;

  /// Kokkos variable, FE type ID, family, and order of the Lagrange multiplier
  Variable _lm_var;
  unsigned int _lm_fe = 0;
  libMesh::FEFamily _lm_family;
  libMesh::Order _lm_order;

  /// Whether the Lagrange multiplier uses a dual basis
  bool _use_dual;

  /// Displacement components of the current configuration, one variable each
  Variable _disp_var[3];
  /// Number of displacement components whose Jacobian columns the constraint assembles
  unsigned int _n_disp = 0;
  /// FE type ID, family, and order of the displacement variables
  unsigned int _disp_fe = 0;
  libMesh::FEFamily _disp_family = libMesh::LAGRANGE;
  libMesh::Order _disp_order = libMesh::FIRST;
  /// The variable numbers coupled to each displacement component, a bit per variable of the system
  Array<bool> _disp_coupled;
  /// Number of variables of the system
  unsigned int _n_vars = 0;

  /// Whether the current dispatch assembles the residual and the Jacobian
  bool _computing_residual = false, _computing_jacobian = false;

private:
  /// Pack the owned Lagrange multiplier degrees of freedom that no mortar segment reaches
  void packInactiveDofs();

  /// The dispatcher of the pinning of the inactive Lagrange multiplier degrees of freedom
  std::unique_ptr<DispatcherBase> _inactive_dispatcher;

  /// Update the geometry and dispatch the loops
  void dispatch(bool residual, bool jacobian);

  /// Whether this constraint assembles the rows of the Lagrange multiplier, and so pins its
  /// inactive degrees of freedom
  const bool _compute_lm_residuals;

  /// Local indices of the owned Lagrange multiplier degrees of freedom that no mortar segment
  /// reaches
  Array<dof_id_type> _inactive_dofs;
  /// The revision of the interface geometry that the inactive degrees of freedom were packed for
  unsigned int _inactive_revision = 0;
};

/**
 * The base class of Kokkos mortar constraints that enforce a constraint between the secondary and
 * the primary side of an interface with a Lagrange multiplier on the lower-dimensional secondary
 * elements. The primal variables are evaluated on the interior parents of the secondary and primary
 * faces. The residual is
 *
 *   R_secondary,i = sum_qp JxW f_s phi_secondary,i
 *   R_primary,i   = sum_qp JxW f_p phi_primary,i
 *   R_lambda,i    = sum_qp JxW f_l psi_i
 *
 * over the quadrature points of the mortar segments, where psi is the basis of the Lagrange
 * multiplier, its dual basis when it uses one. A derived class returns the coefficients f from
 *
 *   Real computeQpResidual(MortarSide side, const MortarDatum & datum) const;
 *
 * and their derivatives with respect to the secondary, primary, and Lagrange multiplier values and
 * the gap from
 *
 *   void computeQpJacobian(MortarSide side, const MortarDatum & datum, Real & d_secondary,
 *                          Real & d_primary, Real & d_lambda, Real & d_gap) const;
 *
 * On the current configuration (use_displaced_mesh = true) the segments come from the mortar
 * interface of the displaced mesh and are repacked before every evaluation. The gap
 * g = (x_primary - x_secondary) . n then depends on the displacements u through
 * x = X + u, and the base assembles the displacement columns
 *
 *   d g / d u_secondary,j = -n phi_secondary,j,  d g / d u_primary,j = n phi_primary,j
 *
 * from d_gap. The normal, the segment quadrature points, and their weights are held fixed in this
 * linearization, as in the non-Kokkos mortar constraints.
 *
 * One team assembles one local secondary element.
 */
class MortarConstraint : public MortarConstraintBase
{
public:
  static InputParameters validParams();

  MortarConstraint(const InputParameters & parameters);
  MortarConstraint(const MortarConstraint & object);

  /// Kokkos function tag of the residual and Jacobian computation, one team per secondary element
  struct MortarLoop
  {
  };

  using MortarConstraintBase::operator();

  /**
   * The team operation called by Kokkos for each local secondary element
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(MortarLoop, const TeamMember<MortarLoop> & team, const Derived & constraint) const;

protected:
  virtual void dispatchMortar(ThreadID n_secondary) override;

  /// The variable on the secondary side
  MooseVariableFieldBase & _secondary_var;
  /// The variable on the primary side
  MooseVariableFieldBase & _primary_var;

private:
  /// Kokkos variables of the sides, indexed by MortarSide
  Variable _side_var[3];
  /// FE type IDs of the variables of the sides
  unsigned int _side_fe[3];
  /// FE families and orders of the variables of the sides
  libMesh::FEFamily _side_family[3];
  libMesh::Order _side_order[3];
  /// Whether the row variable of a side is coupled to the column variable of a side
  bool _coupled[3][3];
};

template <typename Derived>
KOKKOS_FUNCTION void
MortarConstraintBase::operator()(InactiveLoop, const ThreadID tid, const Derived &) const
{
  const auto & var = _lm_var;
  const auto dof = _inactive_dofs[tid];
  const auto lambda = kokkosSystem(var.sys()).getVectorDofValue(dof, var.tag());

  accumulateTaggedDofDiagonal(var.sys(), dof, lambda, 1, _computing_residual, _computing_jacobian);
}

template <typename Derived>
KOKKOS_FUNCTION void
MortarConstraint::operator()(MortarLoop,
                             const TeamMember<MortarLoop> & team,
                             const Derived & constraint) const
{
  constexpr auto S = static_cast<unsigned int>(MortarSide::Secondary);
  constexpr auto P = static_cast<unsigned int>(MortarSide::Primary);
  constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);

  const auto secondary = team.league_rank();
  const auto begin = _geometry.secondary_qp_begin[secondary];
  const auto end = _geometry.secondary_qp_begin[secondary + 1];
  const auto & mesh = kokkosMesh();
  const auto & assembly = kokkosAssembly();

  const auto n_dofs = [&](const unsigned int side, const dof_id_type qp)
  {
    return assembly.getNumDofs(mesh.getElementInfo(_geometry.elem[side][qp]).type, _side_fe[side]);
  };
  const auto lagrange = [&](const unsigned int side, const unsigned int i, const dof_id_type qp)
  { return _geometry.shape(_side_family[side], _side_order[side], side, i, qp); };

  // The dual basis coefficients of the Lagrange multiplier on this secondary element, computed
  // from its segment quadrature with the routine libMesh uses on the host
  const auto n_lambda = n_dofs(L, begin);
  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_side_family[L], _side_order[L], secondary, n_lambda, dual_coeff);

  // The basis function of a side at a point: the dual basis for a Lagrange multiplier using one
  const auto basis = [&](const unsigned int side, const unsigned int i, const dof_id_type qp)
  {
    if (side != L)
      return lagrange(side, i, qp);

    return _geometry.lowerBasis(
        _side_family[L], _side_order[L], _use_dual ? dual_coeff : nullptr, n_lambda, i, qp);
  };

  const auto value = [&](const unsigned int side, const dof_id_type qp)
  {
    const auto & var = _side_var[side];
    const auto & sys = kokkosSystem(var.sys());
    const auto elem = _geometry.elem[side][qp];

    Real u = 0;
    for (unsigned int i = 0; i < n_dofs(side, qp); ++i)
      u += sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var.var()), var.tag()) *
           basis(side, i, qp);
    return u;
  };

  for (dof_id_type qp = begin; qp < end; ++qp)
  {
    const auto & n = _geometry.normal[qp];
    const auto & d = _geometry.gap_vector[qp];
    const MortarDatum datum{
        value(S, qp), value(P, qp), value(L, qp), d(0) * n(0) + d(1) * n(1) + d(2) * n(2)};

    for (unsigned int side = 0; side < 3; ++side)
    {
      const auto row_elem = _geometry.elem[side][qp];
      const auto & row_var = _side_var[side];

      Real f = 0, d[3] = {0, 0, 0}, d_gap = 0;
      if (_computing_residual)
        f = constraint.computeQpResidual(static_cast<MortarSide>(side), datum);
      if (_computing_jacobian)
        constraint.computeQpJacobian(static_cast<MortarSide>(side), datum, d[S], d[P], d[L], d_gap);

      // The threads of the team split the test functions of the side
      ::Kokkos::parallel_for(
          ::Kokkos::TeamThreadRange(team, n_dofs(side, qp)),
          [&](const unsigned int i)
          {
            const auto w = _geometry.JxW[qp] * basis(side, i, qp);

            if (_computing_residual)
              accumulateTaggedResidual(w * f, row_var, row_elem, i);

            if (_computing_jacobian)
            {
              for (unsigned int col = 0; col < 3; ++col)
                if (d[col] && _coupled[side][col])
                  for (unsigned int j = 0; j < n_dofs(col, qp); ++j)
                    accumulateTaggedMatrix(w * d[col] * basis(col, j, qp),
                                           row_var,
                                           row_elem,
                                           i,
                                           _side_var[col],
                                           _geometry.elem[col][qp],
                                           j);

              // The gap moves with the displacements of both faces
              if (d_gap)
                for (unsigned int c = 0; c < _n_disp; ++c)
                  if (displacementCoupled(row_var.var(), c))
                  {
                    const auto coef = w * d_gap * _geometry.normal[qp](c);
                    accumulateDisplacementJacobian(-coef, row_var, row_elem, i, c, S, qp);
                    accumulateDisplacementJacobian(coef, row_var, row_elem, i, c, P, qp);
                  }
            }
          });
    }
  }
}

KOKKOS_FUNCTION inline bool
MortarConstraintBase::displacementCoupled(const unsigned int row, const unsigned int c) const
{
  return _disp_coupled[_n_vars * c + row];
}

KOKKOS_FUNCTION inline void
MortarConstraintBase::accumulateDisplacementJacobian(const Real coef,
                                                     const Variable & row_var,
                                                     const ContiguousElementID row_elem,
                                                     const unsigned int i,
                                                     const unsigned int c,
                                                     const unsigned int face,
                                                     const dof_id_type qp) const
{
  if (!coef)
    return;

  // A face whose block the displacement does not span stays in place
  const auto elem = _geometry.elem[face][qp];
  const auto & info = kokkosMesh().getElementInfo(elem);
  if (!kokkosSystem(_disp_var[c].sys()).isVariableActive(_disp_var[c].var(), info.subdomain))
    return;

  const auto n_disp_dofs = kokkosAssembly().getNumDofs(info.type, _disp_fe);
  for (unsigned int j = 0; j < n_disp_dofs; ++j)
    accumulateTaggedMatrix(coef * _geometry.shape(_disp_family, _disp_order, face, j, qp),
                           row_var,
                           row_elem,
                           i,
                           _disp_var[c],
                           elem,
                           j);
}

KOKKOS_FUNCTION inline void
MortarConstraintBase::accumulateNormalJacobian(const Real3 & coef,
                                               const Variable & row_var,
                                               const ContiguousElementID row_elem,
                                               const unsigned int i,
                                               const dof_id_type normal) const
{
  for (auto k = _geometry.sensitivity_begin[normal]; k < _geometry.sensitivity_begin[normal + 1];
       ++k)
  {
    const auto & dn_dx = _geometry.sensitivity[k];
    const auto elem = _geometry.sensitivity_elem[k];
    const auto node = _geometry.sensitivity_elem_node[k];

    for (unsigned int c = 0; c < _n_disp; ++c)
    {
      if (!displacementCoupled(row_var.var(), c))
        continue;

      Real d = 0;
      for (unsigned int n = 0; n < 3; ++n)
        d += coef(n) * dn_dx(n, c);

      // The displacement is nodal, so its degree of freedom at the node is the one of the element
      // shape function of that node
      accumulateTaggedMatrix(d, row_var, row_elem, i, _disp_var[c], elem, node);
    }
  }
}

} // namespace Moose::Kokkos

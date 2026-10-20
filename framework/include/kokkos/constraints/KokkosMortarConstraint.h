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
#include "MortarConsumerInterface.h"
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
 * One team assembles one local secondary element. The equation of each Lagrange multiplier degree
 * of freedom that no mortar segment reaches becomes lambda = 0.
 */
class MortarConstraint : public ResidualObject,
                         public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  MortarConstraint(const InputParameters & parameters);
  MortarConstraint(const MortarConstraint & object);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /// The side whose test functions a residual coefficient multiplies
  using MortarSide = Moose::Kokkos::MortarSide;

  /// Kokkos function tag of the residual and Jacobian computation, one team per secondary element
  struct MortarLoop
  {
  };

  /// Kokkos function tag of the pinning of the inactive Lagrange multiplier degrees of freedom
  struct InactiveLoop
  {
  };

  /**
   * The team operation called by Kokkos for each local secondary element
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(MortarLoop, const TeamMember<MortarLoop> & team, const Derived & constraint) const;

  /**
   * The operation called by Kokkos for each inactive Lagrange multiplier degree of freedom
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(InactiveLoop, const ThreadID tid, const Derived &) const;

  /// Maximum number of shape functions of a variable on a lower-dimensional element
  static constexpr unsigned int max_shapes = 9;

protected:
  /// The variable on the secondary side
  MooseVariableFieldBase & _secondary_var;
  /// The variable on the primary side
  MooseVariableFieldBase & _primary_var;

private:
  /**
   * The host-side mortar interface of the constraint, which the device copies of the constraint do
   * not carry
   */
  class Interface : public MortarConsumerInterface
  {
  public:
    Interface(const MooseObject * object) : MortarConsumerInterface(object) {}

    using MortarConsumerInterface::amg;
  };

  /// The host-side mortar interface, shared by the copies of the constraint
  std::shared_ptr<Interface> _interface;

  /// The geometry of the interface, shared by the Kokkos objects posed on it
  MortarInterfaceData & _interface_data;

  /// Pack the owned Lagrange multiplier degrees of freedom that no mortar segment reaches
  void packInactiveDofs();

  /// The dispatcher of the pinning of the inactive Lagrange multiplier degrees of freedom
  std::unique_ptr<DispatcherBase> _inactive_dispatcher;

  /// Dispatch the mortar loop
  void dispatch(bool residual, bool jacobian);

  /// Kokkos variables of the sides, indexed by MortarSide
  Variable _side_var[3];
  /// FE type IDs of the variables of the sides
  unsigned int _side_fe[3];
  /// FE families and orders of the variables of the sides
  libMesh::FEFamily _side_family[3];
  libMesh::Order _side_order[3];
  /// Whether the row variable of a side is coupled to the column variable of a side
  bool _coupled[3][3];

  /// Displacement components of the current configuration, one variable each
  Variable _disp_var[3];
  /// Number of displacement components whose Jacobian columns the constraint assembles
  unsigned int _n_disp = 0;
  /// FE type ID, family, and order of the displacement variables
  unsigned int _disp_fe = 0;
  libMesh::FEFamily _disp_family = libMesh::LAGRANGE;
  libMesh::Order _disp_order = libMesh::FIRST;
  /// Whether the row variable of a side is coupled to a displacement component
  bool _disp_coupled[3][3];

  /// Whether the Lagrange multiplier uses a dual basis
  bool _use_dual;

  /// Whether the current dispatch assembles the residual and the Jacobian
  bool _computing_residual = false, _computing_jacobian = false;

  /// The geometry of the interface as of the current dispatch
  MortarInterfaceGeometry _geometry;

  /// Local indices of the owned Lagrange multiplier degrees of freedom that no mortar segment
  /// reaches
  Array<dof_id_type> _inactive_dofs;
  /// The revision of the interface geometry that the inactive degrees of freedom were packed for
  unsigned int _inactive_revision = 0;
};

template <typename Derived>
KOKKOS_FUNCTION void
MortarConstraint::operator()(InactiveLoop, const ThreadID tid, const Derived &) const
{
  const auto & var = _side_var[static_cast<unsigned int>(MortarSide::Lower)];
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
  {
    const auto & xi = _geometry.xi[side][qp];
    const libMesh::FEShapeKey key{_side_family[side], _geometry.type[side][qp], _side_order[side]};
    return libMesh::Kokkos::shape(key, i, xi(0), xi(1), xi(2));
  };
  const auto disp_shape = [&](const unsigned int side, const unsigned int j, const dof_id_type qp)
  {
    const auto & xi = _geometry.xi[side][qp];
    const libMesh::FEShapeKey key{_disp_family, _geometry.type[side][qp], _disp_order};
    return libMesh::Kokkos::shape(key, j, xi(0), xi(1), xi(2));
  };

  // The dual basis coefficients of the Lagrange multiplier on this secondary element, computed
  // from its segment quadrature with the routine libMesh uses on the host
  const auto n_lambda = n_dofs(L, begin);
  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
  {
    Real A[max_shapes][max_shapes], D[max_shapes][max_shapes], rhs[max_shapes], x[max_shapes];
    auto A_ref = [&A](const unsigned int i, const unsigned int j) -> Real & { return A[i][j]; };
    auto D_ref = [&D](const unsigned int i, const unsigned int j) -> Real & { return D[i][j]; };
    auto rhs_ref = [&rhs](const unsigned int i) -> Real & { return rhs[i]; };
    auto x_ref = [&x](const unsigned int i) -> Real & { return x[i]; };
    auto coeff_ref = [&dual_coeff](const unsigned int i, const unsigned int j) -> Real &
    { return dual_coeff[i][j]; };
    const auto JxW = [&](const unsigned int qp) { return _geometry.JxW[begin + qp]; };
    const auto phi = [&](const unsigned int i, const unsigned int qp)
    { return lagrange(L, i, begin + qp); };

    libMesh::compute_dual_coeffs(_side_family[L],
                                 _geometry.type[L][begin],
                                 n_lambda,
                                 end - begin,
                                 JxW,
                                 phi,
                                 A_ref,
                                 D_ref,
                                 rhs_ref,
                                 x_ref,
                                 coeff_ref);
  }

  // The basis function of a side at a point: the dual basis for a Lagrange multiplier using one
  const auto basis = [&](const unsigned int side, const unsigned int i, const dof_id_type qp)
  {
    if (side != L || !_use_dual)
      return lagrange(side, i, qp);

    Real psi = 0;
    for (unsigned int k = 0; k < n_lambda; ++k)
      psi += dual_coeff[k][i] * lagrange(L, k, qp);
    return psi;
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
                  if (_disp_coupled[side][c])
                    for (const unsigned int face : {S, P})
                    {
                      // A face whose block the displacement does not span stays in place
                      const auto elem = _geometry.elem[face][qp];
                      const auto & info = mesh.getElementInfo(elem);
                      if (!kokkosSystem(_disp_var[c].sys())
                               .isVariableActive(_disp_var[c].var(), info.subdomain))
                        continue;
                      const auto n_disp_dofs = assembly.getNumDofs(info.type, _disp_fe);
                      const auto sign = face == S ? -1 : 1;
                      for (unsigned int j = 0; j < n_disp_dofs; ++j)
                        accumulateTaggedMatrix(w * d_gap * sign * _geometry.normal[qp](c) *
                                                   disp_shape(face, j, qp),
                                               row_var,
                                               row_elem,
                                               i,
                                               _disp_var[c],
                                               elem,
                                               j);
                    }
            }
          });
    }
  }
}

} // namespace Moose::Kokkos

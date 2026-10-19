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
 * and their derivatives with respect to the secondary, primary, and Lagrange multiplier values
 * from
 *
 *   void computeQpJacobian(MortarSide side, const MortarDatum & datum, Real & d_secondary,
 *                          Real & d_primary, Real & d_lambda) const;
 *
 * One team assembles one local secondary element.
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
  enum class MortarSide : unsigned int
  {
    Secondary = 0,
    Primary = 1,
    Lower = 2
  };

  /// Kokkos function tag of the residual and Jacobian computation, one team per secondary element
  struct MortarLoop
  {
  };

  /**
   * The team operation called by Kokkos for each local secondary element
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(MortarLoop, const TeamMember<MortarLoop> & team, const Derived & constraint) const;

  /// Maximum number of shape functions of a variable on a lower-dimensional element
  static constexpr unsigned int max_shapes = 9;

protected:
  /// The variable on the secondary side
  MooseVariableFieldBase & _secondary_var;
  /// The variable on the primary side
  MooseVariableFieldBase & _primary_var;

private:
  /**
   * The host-side mortar interface state of the constraint, which the device copies of the
   * constraint do not carry: the mortar interface, and the notification that its segment mesh
   * coverage changed
   */
  class Interface : public MortarConsumerInterface, public MortarExecutorInterface
  {
  public:
    Interface(const MooseObject * object, FEProblemBase & problem);

    virtual void mortarSetup(const AutomaticMortarGeneration &) override { needs_packing = true; }

    using MortarConsumerInterface::amg;

    /// Whether the segment data must be repacked before the next dispatch
    bool needs_packing = true;
  };

  /// The host-side mortar interface state, shared by the copies of the constraint
  std::shared_ptr<Interface> _interface;

  /// Pack the mortar segment quadrature points of the local secondary elements
  void packSegments();

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

  /// Whether the Lagrange multiplier uses a dual basis
  bool _use_dual;

  /// Whether the current dispatch assembles the residual and the Jacobian
  bool _computing_residual = false, _computing_jacobian = false;

  /**
   * Mortar segment quadrature point data, ordered by local secondary element
   */
  ///@{
  /// Range of the quadrature points of each local secondary element
  Array<dof_id_type> _secondary_qp_begin;
  /// Contiguous ID of the element of each side at each point: the interior parents of the
  /// secondary and primary faces, on which the primal variables live, and the lower-dimensional
  /// secondary element, on which the Lagrange multiplier lives
  Array<ContiguousElementID> _elem[3];
  /// Element type of the element of each side at each point
  Array<libMesh::ElemType> _type[3];
  /// Reference coordinates of each point on the element of each side
  Array<Real3> _xi[3];
  /// Quadrature weight of each point, including the coordinate transform factor
  Array<Real> _JxW;
  /// Gap at each point: the distance from the secondary to the primary face along the interpolated
  /// secondary normal
  Array<Real> _gap;
  ///@}
};

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
  const auto begin = _secondary_qp_begin[secondary];
  const auto end = _secondary_qp_begin[secondary + 1];
  const auto & mesh = kokkosMesh();
  const auto & assembly = kokkosAssembly();

  const auto n_dofs = [&](const unsigned int side, const dof_id_type qp)
  { return assembly.getNumDofs(mesh.getElementInfo(_elem[side][qp]).type, _side_fe[side]); };
  const auto lagrange = [&](const unsigned int side, const unsigned int i, const dof_id_type qp)
  {
    const auto & xi = _xi[side][qp];
    const libMesh::FEShapeKey key{_side_family[side], _type[side][qp], _side_order[side]};
    return libMesh::Kokkos::shape(key, i, xi(0), xi(1), xi(2));
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
    const auto JxW = [&](const unsigned int qp) { return _JxW[begin + qp]; };
    const auto phi = [&](const unsigned int i, const unsigned int qp)
    { return lagrange(L, i, begin + qp); };

    libMesh::compute_dual_coeffs(_side_family[L],
                                 _type[L][begin],
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
    const auto elem = _elem[side][qp];

    Real u = 0;
    for (unsigned int i = 0; i < n_dofs(side, qp); ++i)
      u += sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var.var()), var.tag()) *
           basis(side, i, qp);
    return u;
  };

  for (dof_id_type qp = begin; qp < end; ++qp)
  {
    const MortarDatum datum{value(S, qp), value(P, qp), value(L, qp), _gap[qp]};

    for (unsigned int side = 0; side < 3; ++side)
    {
      const auto row_elem = _elem[side][qp];
      const auto & row_var = _side_var[side];

      Real f = 0, d[3] = {0, 0, 0};
      if (_computing_residual)
        f = constraint.computeQpResidual(static_cast<MortarSide>(side), datum);
      if (_computing_jacobian)
        constraint.computeQpJacobian(static_cast<MortarSide>(side), datum, d[S], d[P], d[L]);

      // The threads of the team split the test functions of the side
      ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_dofs(side, qp)),
                             [&](const unsigned int i)
                             {
                               const auto w = _JxW[qp] * basis(side, i, qp);

                               if (_computing_residual)
                                 accumulateTaggedResidual(w * f, row_var, row_elem, i);

                               if (_computing_jacobian)
                                 for (unsigned int col = 0; col < 3; ++col)
                                   if (d[col] && _coupled[side][col])
                                     for (unsigned int j = 0; j < n_dofs(col, qp); ++j)
                                       accumulateTaggedMatrix(w * d[col] * basis(col, j, qp),
                                                              row_var,
                                                              row_elem,
                                                              i,
                                                              _side_var[col],
                                                              _elem[col][qp],
                                                              j);
                             });
    }
  }
}

} // namespace Moose::Kokkos

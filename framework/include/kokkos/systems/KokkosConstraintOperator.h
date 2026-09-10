//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"
#include "KokkosThread.h"
#include "KokkosTypes.h"
#include "KokkosVector.h"

#include <unordered_map>

namespace libMesh
{
class DofMap;
template <typename>
class NumericVector;
}

namespace Moose::Kokkos
{

/**
 * Device-resident mirror of the rows of a libMesh DofMap's DOF constraints, built with no
 * per-object attribution: every row DofMap::get_dof_constraints() reports is taken as-is, whatever
 * produced it. For a Kokkos matrix-free system this is currently always a Dirichlet-type nodal
 * boundary condition's row, registered as a libMesh DirichletBoundary, so this operator is what
 * lets a boundary condition's device dispatch stop enforcing its own DOFs by boundary node and
 * instead simply mirror whichever DOFs libMesh itself reports as constrained, vertex and bubble
 * (e.g. HIERARCHIC edge/face mode) alike.
 *
 * One instance is built per DofSpace: the fine system's own layout, and each p-multigrid level's
 * layout, each against its own libMesh::DofMap. A level only ever reads constrainedMask(), since a
 * level carries no nonlinear solve of its own; the fine system additionally uses the preset,
 * residual, Jacobian-vector product and diagonal finalizers below to enforce the constraint during
 * the nonlinear solve itself.
 *
 * Every row this operator currently constrains is empty (a pure Dirichlet pin has no cross-DOF
 * coupling; see ConstrainDirichlet::apply_dirichlet_impl in libMesh's dof_map_constraints.C, whose
 * committed DofConstraintRow is always empty even after a HIERARCHIC edge/face mode is projected).
 * The CSR layout below and the coupling terms in the device kernels are still expressed in general
 * form so that a future coupled constraint (e.g. KokkosMatchedValueBC's DofMap::add_constraint_row)
 * needs no change to this class, only a non-empty row to exercise.
 */
class ConstraintOperator
{
public:
  ConstraintOperator() = default;

  /**
   * Build the operator from a DofMap's current constraint rows
   * @param dof_map The libMesh DOF map whose constraints this operator mirrors
   * @param solution A vector over the same DofMap, used to map a global DOF id to the local(+ghost)
   * index the Kokkos DOF layout indexes by
   * @param num_local_plus_ghost The size of the local-plus-ghost DOF index space, which sizes the
   * dense constrained-row mask
   * @param preset_dofs Map from a global DOF id a registered Dirichlet-type boundary condition
   * covers to whether that boundary condition presets its value ahead of the nonlinear solve. A
   * constrained row with no entry here (not currently possible for a Kokkos matrix-free system,
   * which errors out on any other source of a DOF constraint) defaults to non-preset.
   */
  void setup(const libMesh::DofMap & dof_map,
             libMesh::NumericVector<Number> & solution,
             dof_id_type num_local_plus_ghost,
             const std::unordered_map<dof_id_type, bool> & preset_dofs);

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the number of rows this operator constrains, on this process
   * @returns The number of constrained rows
   */
  dof_id_type numConstrainedDofs() const { return _rows.size(); }

  /**
   * Get the local-plus-ghost mask of the rows this operator constrains, which a level's operator
   * skips when contracting its own action, diagonal or assembled matrix
   * @returns The mask, indexed by local DOF index
   */
  const Array<bool> & constrainedMask() const { return _constrained_mask; }

  /**
   * Set every row this operator presets, in a tagged vector, to the value libMesh's constraint
   * machinery reports for it: g_i + sum_j c_ij * x_j. A non-preset row is left alone, since the
   * nonlinear solve solves for it through its own residual/Jacobian row instead.
   * @param solution The vector to preset
   */
  void presetSolution(Vector & solution);

  /**
   * Set every row this operator constrains, in a tagged vector, to the value libMesh's constraint
   * machinery reports for it, whether or not the row is preset.
   *
   * This is applied to the solution the quadrature-point cache is built from, ahead of every
   * linearization, so that no kernel ever contracts a constrained DOF's raw iterate value: a free
   * row's equation is a function of the free DOFs and of the constrained values as data, never of
   * the constrained DOFs as unknowns. That is what makes a free row's derivative with respect to a
   * constrained DOF zero, which in turn is what makes the matrix-free action symmetric whenever the
   * physics is -- the constrained row's own action is already the identity, so the transposed entry
   * it would have to match is zero.
   *
   * Consistency between the residual and its derivative is the reason this belongs on the gathered
   * solution rather than on the constrained rows of the action alone: a finite difference of the
   * residual with respect to a constrained DOF only comes out zero, matching the action, if the
   * residual genuinely does not read that DOF.
   *
   * @param solution The vector to write the constrained values into
   */
  void enforceConstraints(Vector & solution);

  /**
   * Finalize the residual vector at every row this operator constrains, after the ordinary kernel
   * and boundary condition residual sweep has run, to its own constraint equation,
   * u_i - g_i - sum_j c_ij u_j -- the same row-replacement form NodalBC's own per-node dispatch
   * already uses for a vertex DOF, with no distinction between a preset and a non-preset row. A
   * preset row's solution already satisfies this exactly (presetSolution() put it there), so the
   * row's value comes out zero; a non-preset row's value is a real, nonzero residual the solve
   * still has to drive to zero. Either way the row's derivative is the identity
   * finalizeJacobianVectorProduct() and finalizeDiagonal() below give it, which is what lets Newton
   * leave a preset row's solution untouched without this residual formula needing to single it out.
   * @param residual The residual vector to finalize
   * @param solution The current solution vector the row's own equation reads
   */
  void finalizeResidual(Vector & residual, const Vector & solution);

  /**
   * Finalize the Kokkos matrix-free action vector at every row this operator constrains, after the
   * ordinary operator/kernel action has run: the row's action becomes its own constraint
   * equation's directional derivative, x_i - sum_j c_ij x_j, with no distinction between a preset
   * and a non-preset row. Only this row's own action is replaced; a free row's action is already
   * free of this DOF's direction, because the operator leaves a constrained DOF out of its
   * trial-space gather (see QpJacobianOperator's ApplyLoop) exactly as enforceConstraints() leaves
   * it out of the residual, which is what makes the two sides of the transpose agree at zero.
   * @param y The action vector to finalize
   * @param x The direction vector
   */
  void finalizeJacobianVectorProduct(Vector & y, const Vector & x);

  /**
   * Finalize the Kokkos matrix-free diagonal vector at every row this operator constrains, after
   * the ordinary operator/kernel diagonal sweep has run, to the constrained-row identity value the
   * tag being computed carries (1 for the primary Jacobian tag)
   * @param diagonal The diagonal vector to finalize
   * @param value The constrained-row diagonal value for the tag being computed
   */
  void finalizeDiagonal(Vector & diagonal, Real value);

  /**
   * Kokkos function tags for the loops over this operator's constrained rows
   */
  ///@{
  struct ConstrainLoop
  {
  };
  struct ResidualLoop
  {
  };
  struct JVPLoop
  {
  };
  struct DiagonalLoop
  {
  };
  ///@}

  KOKKOS_FUNCTION void operator()(ConstrainLoop, ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(ResidualLoop, ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(JVPLoop, ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(DiagonalLoop, ThreadID tid) const;
#endif

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Write the constrained value of every row _preset_only admits into a tagged vector, the shared
   * body of presetSolution() and enforceConstraints()
   * @param solution The vector to write the constrained values into
   */
  void constrainSolution(Vector & solution);
#endif

  /**
   * Local DOF index of each constrained row
   */
  Array<dof_id_type> _rows;
  /**
   * CSR row pointer into _cols/_coeffs, size _rows.size() + 1
   */
  Array<dof_id_type> _row_offsets;
  /**
   * Local DOF index of each coupled free DOF, flattened over all rows
   */
  Array<dof_id_type> _cols;
  /**
   * Coupling coefficient c_ij, flattened over all rows
   */
  Array<Real> _coeffs;
  /**
   * Inhomogeneity g_i of each row, 0 when the row has none
   */
  Array<Real> _inhomogeneity;
  /**
   * Whether each row is preset, i.e. whether its value is pinned in the solution ahead of the
   * nonlinear solve rather than solved for through its own residual/Jacobian row
   */
  Array<bool> _row_preset;
  /**
   * Local-plus-ghost mask of the rows this operator constrains
   */
  Array<bool> _constrained_mask;
  /**
   * Whether ConstrainLoop is to write only the rows _row_preset marks, which is what distinguishes
   * presetSolution() from enforceConstraints()
   */
  bool _preset_only = false;
  /**
   * Vectors and scalar the loop being dispatched acts on
   */
  ///@{
  Vector _vector;
  Vector _vector2;
  Real _value = 0;
  ///@}
};

} // namespace Moose::Kokkos

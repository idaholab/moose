//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosEntityBlocks.h"
#include "KokkosQpJacobianLevel.h"
#include "KokkosQpJacobianCache.h"
#include "KokkosLevelBasisTable.h"
#include "KokkosMatrix.h"
#include "KokkosThread.h"

namespace Moose::Kokkos
{

#ifdef MOOSE_KOKKOS_SCOPE

/**
 * The action of the linearized operator of one level of a p-multigrid hierarchy, and its diagonal.
 *
 * The operator is the quadrature-point Jacobian cache the fine level fills, contracted against the
 * basis tables a level carries: the direction vector is interpolated to each quadrature point, the
 * cached tensor turns that value/gradient pair into the linearized value/flux pair, and the result
 * is contracted back onto the level's test functions. Which level the loops run on is a
 * constructor argument, so the fine level's matrix-free Jacobian-vector product and a coarse
 * level's smoother operator are the same code with a different level handed to it.
 *
 * The physics is never re-entered: a level that shares the fine quadrature rule needs only the
 * cached tensors and its own basis tables, which is what allows a coarse operator to be Galerkin
 * without an assembled coarse matrix.
 *
 * Where the level's basis factors into a product of one-dimensional shape functions over a
 * quadrature rule that factors the same way, the action is contracted one reference coordinate at a
 * time instead, which is sum factorization. That replaces the dense contraction against the
 * two-dimensional tables with a sequence of contractions against the one-dimensional ones, costing
 * a factor of the mode count fewer operations over a table small enough to stay cached. The two
 * loops partition the (element, variable) pairs by whether the pair's basis factors, so a mesh
 * carrying both kinds of element runs both.
 *
 * The same contraction also assembles a level into a sparse matrix, entry by entry, which is how
 * the coarsest level reaches an algebraic-multigrid coarse solve while remaining the exact Galerkin
 * operator of the fine linearization.
 */
class QpJacobianOperator : public AssemblyHolder
{
public:
  /**
   * Constructor
   * @param assembly The Kokkos assembly holding the quadrature rule and the cached basis tables
   * @param cache The quadrature-point Jacobian cache, which must hold a valid linearization
   * @param level The level the operator acts on
   */
  QpJacobianOperator(const Assembly & assembly,
                     const QpJacobianCache & cache,
                     const QpJacobianLevel & level);

  /**
   * Kokkos function tags for the loops over (element, variable) pairs
   */
  ///@{
  struct ApplyFlopLoop
  {
  };
  struct ApplyScratchLoop
  {
  };
  struct ApplyTeamLoop
  {
  };
  struct ApplyTensorFlopLoop
  {
  };
  struct ApplyTensorSizeLoop
  {
  };
  struct ApplyTensorLoop
  {
  };
  struct DiagonalLoop
  {
  };
  struct MatrixLoop
  {
  };
  struct MatrixIdentityLoop
  {
  };
  struct BlockLoop
  {
  };
  struct BlockScratchLoop
  {
  };
  struct BlockApplyLoop
  {
  };
  struct BlockIdentityLoop
  {
  };
  ///@}

  /**
   * Accumulate the operator's action on a direction vector into an action vector
   * @param x_tag The vector tag of the direction vector on the level
   * @param y_tag The vector tag of the action vector on the level
   */
  void apply(TagID x_tag, TagID y_tag);

  /**
   * Accumulate the operator's diagonal into a vector
   * @param diag_tag The vector tag of the diagonal on the level
   */
  void diagonal(TagID diag_tag);

  /**
   * Accumulate the operator's entries into a matrix over the level's DOF layout, and place the
   * identity on every row the level holds fixed, which together are the operator the level's
   * matrix-free action realizes
   * @param matrix The matrix on the level
   */
  void assemble(Matrix & matrix);

  /**
   * Accumulate the operator's entity-diagonal blocks into an entity-block decomposition, which is
   * the same contraction the assembly performs, restricted to the pairs of degrees of freedom that
   * share a mesh entity
   * @param blocks The decomposition, which must already be built and zeroed
   */
  void assembleBlocks(const EntityBlocks & blocks);

  /**
   * Apply the inverse of every entity block to a residual vector, which is the entity-block
   * smoother
   *
   * A row belonging to no block is a row the level holds fixed, where the operator carries the
   * identity, so the residual is copied to those rows and the smoother carries the identity too.
   * The blocks are disjoint, so their applications are independent and the result is the additive
   * smoother over the decomposition.
   *
   * @param blocks The decomposition, whose blocks must already be factored
   * @param r_tag The vector tag of the residual on the level
   * @param x_tag The vector tag of the correction on the level
   */
  void applyBlocks(const EntityBlocks & blocks, TagID r_tag, TagID x_tag);

  KOKKOS_FUNCTION void operator()(ApplyFlopLoop, const ThreadID tid, double & flops) const;
  KOKKOS_FUNCTION void operator()(ApplyScratchLoop, const ThreadID tid, std::size_t & bytes) const;

  KOKKOS_FUNCTION void operator()(ApplyTensorFlopLoop, const ThreadID tid, double & flops) const;
  KOKKOS_FUNCTION void
  operator()(ApplyTensorSizeLoop, const ThreadID tid, unsigned int & team_size) const;

  /// The team policies apply() runs under, one team per (element, variable) pair
  ///@{
  using ApplyPolicy = ::Kokkos::TeamPolicy<ExecSpace, ApplyTeamLoop>;
  using ApplyTeam = ApplyPolicy::member_type;
  using ApplyTensorPolicy = ::Kokkos::TeamPolicy<ExecSpace, ApplyTensorLoop>;
  using ApplyTensorTeam = ApplyTensorPolicy::member_type;
  ///@}

  /// The team policy assembleBlocks() runs under, one team per (element, variable) pair
  ///@{
  using BlockPolicy = ::Kokkos::TeamPolicy<ExecSpace, BlockLoop>;
  using BlockTeam = BlockPolicy::member_type;
  ///@}

  KOKKOS_FUNCTION void operator()(ApplyTeamLoop, const ApplyTeam & team) const;
  KOKKOS_FUNCTION void operator()(ApplyTensorLoop, const ApplyTensorTeam & team) const;
  KOKKOS_FUNCTION void operator()(DiagonalLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(MatrixIdentityLoop, const dof_id_type dof) const;
  KOKKOS_FUNCTION void operator()(BlockLoop, const BlockTeam & team) const;
  KOKKOS_FUNCTION void
  operator()(BlockScratchLoop, const ThreadID tid, std::size_t & bytes) const;
  KOKKOS_FUNCTION void operator()(BlockApplyLoop, const dof_id_type block) const;
  KOKKOS_FUNCTION void operator()(BlockIdentityLoop, const dof_id_type dof) const;

private:
  /**
   * The floating point operations one apply() performs on the pairs of each of its two loops,
   * counted once on first use and reused after. A loop whose count is zero owns no pair and is not
   * dispatched. Negative means they have not been counted yet.
   */
  ///@{
  double _apply_flops = -1;
  double _apply_tensor_flops = -1;
  ///@}

  /**
   * The per-team scratch an apply() needs, being the largest any (element, variable) pair asks for
   * on whichever of the two loops owns it. Both loops are given that much, since the difference
   * between them is far below what limits occupancy. Zero means it has not been measured yet.
   */
  std::size_t _apply_scratch_bytes = 0;

  /**
   * The per-team scratch an assembleBlocks() needs, being the largest any (element, variable) pair
   * asks for. Zero means it has not been measured yet.
   */
  std::size_t _block_scratch_bytes = 0;

  /**
   * The team size the sum-factorized apply() runs under, being the largest range any phase of that
   * loop runs over, so that every phase has a thread per index. Leaving the size to
   * ::Kokkos::AUTO instead gives a team whose threads beyond that range do nothing. Zero means it
   * has not been measured yet.
   */
  unsigned int _apply_tensor_team_size = 0;

  /**
   * The quadrature-point linearization the operator contracts
   */
  const QpJacobianCache _cache;

  /**
   * The level whose basis tables and DOF layout the contraction runs on
   */
  const QpJacobianLevel _level;

  /**
   * Kokkos thread object over the (element, variable) pairs of the loops
   */
  Thread<> _thread;

  /**
   * Vector tags of the direction and action vectors of the loop being dispatched
   */
  ///@{
  TagID _x_tag = 0;
  TagID _y_tag = 0;
  ///@}

  /**
   * The matrix of the assembly loop, over the level's DOF layout
   */
  Matrix _matrix;

  /**
   * The entity-block decomposition of the block loops, over the level's DOF layout
   */
  EntityBlocks _blocks;
};

#endif

} // namespace Moose::Kokkos

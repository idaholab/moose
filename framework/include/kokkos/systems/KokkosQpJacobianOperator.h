//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
  struct ApplyLoop
  {
  };
  struct DiagonalLoop
  {
  };
  struct MatrixLoop
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
   * Accumulate the operator's entries into a matrix over the level's DOF layout
   * @param matrix The matrix on the level
   */
  void assemble(Matrix & matrix);

  KOKKOS_FUNCTION void operator()(ApplyLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(DiagonalLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid) const;

private:
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
};

#endif

} // namespace Moose::Kokkos

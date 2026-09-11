//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDofSpace.h"

#include "libmesh/system.h"

#ifdef MOOSE_KOKKOS_SCOPE

class ConsoleStream;

namespace Moose::Kokkos
{

/**
 * One side of a level pair as a transfer sees it: the libMesh system the transfer's work vectors
 * and parallel ghosting come from, the device DOF layout its element loops index, the FE type each
 * variable carries on that side, and the DOFs the side holds fixed.
 *
 * A side is a level of a p-multigrid hierarchy, and the finest side is the solver system itself, so
 * this bundles what the two have in common as function spaces.
 */
struct TransferSide
{
  /// The libMesh system the side's vectors and ghosting come from
  libMesh::System & sys;
  /// The device DOF layout the side's vectors are indexed by
  const DofSpace & dof_space;
  /// The FE type ID each variable carries on the side
  Array<unsigned int> fe_types;
  /// Local-plus-ghost mask of the side's DOFs a nodal boundary condition holds fixed, which is
  /// unallocated when the side holds none fixed
  Array<bool> constrained_dof;
};

/**
 * Transfer of a vector between two levels of a p-multigrid hierarchy.
 *
 * A transfer supplies prolongation, P, and restriction, which is exactly the transpose of that same
 * P, so the operator the hierarchy realizes on a coarse level is the Galerkin operator P^T J P of
 * the fine linearization. Transfers are a function of the geometry and the bases alone, so one is
 * built once at setup and reused across every Newton step.
 *
 * A transfer is one implementation behind this interface, and consumers program against the
 * interface: an implementation may be an index-set scatter, for a family whose coarse space is
 * spanned by a subset of the fine basis functions; a per-element embedding, for a family whose
 * nested spaces share no degrees of freedom; or a projection, for level pairs that are nested in
 * neither sense. A consumer therefore treats a transfer as a general linear operator between the
 * two sides.
 */
class LevelTransfer
{
public:
  /**
   * Constructor
   * @param coarse The coarse side of the level pair
   * @param fine The fine side of the level pair
   */
  LevelTransfer(const TransferSide & coarse, const TransferSide & fine)
    : _coarse(coarse), _fine(fine)
  {
  }

  virtual ~LevelTransfer() = default;

  /**
   * Prolong a coarse vector to the fine side, y = P x
   * @param x The coarse vector
   * @param y The fine vector
   */
  virtual void prolong(const libMesh::NumericVector<Number> & x,
                       libMesh::NumericVector<Number> & y) = 0;

  /**
   * Restrict a fine vector to the coarse side, y = P^T x
   * @param x The fine vector
   * @param y The coarse vector
   */
  virtual void restrictTo(const libMesh::NumericVector<Number> & x,
                          libMesh::NumericVector<Number> & y) = 0;

  /**
   * Check that restriction is the transpose of prolongation, which is what makes the operator the
   * hierarchy realizes on the coarse side the Galerkin operator of the fine linearization, and
   * report the agreement. Errors out on a mismatch.
   * @param console The stream the agreement is reported on
   */
  virtual void verify(const ConsoleStream & console);

  /**
   * Fill a vector with a deterministic, non-constant pattern of its global DOF indices, so that a
   * verification is reproducible from run to run
   * @param vector The vector to fill
   * @param sys The system the vector belongs to
   */
  static void fillVerificationPattern(libMesh::NumericVector<Number> & vector,
                                      const libMesh::System & sys);

protected:
  /// The coarse side of the level pair
  TransferSide _coarse;

  /// The fine side of the level pair
  TransferSide _fine;
};

} // namespace Moose::Kokkos

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MoosePreconditioner.h"

#ifdef MOOSE_KOKKOS_ENABLED

#include "KokkosPLevelSpace.h"

/**
 * p-multigrid preconditioner for the Kokkos matrix-free partial-assembly Jacobian.
 *
 * Every level shares the fine level's quadrature rule, so every level's operator is the same
 * quadrature-point Jacobian cache, filled once per linearization on the fine level, contracted
 * against that level's own basis tables. A coarse level therefore holds no residual objects: it is
 * a function space (Moose::Kokkos::PLevelSpace) and nothing more, which is why the levels are built
 * here at preconditioner-construction time rather than declared as systems in the input.
 */
class PMultigrid : public MoosePreconditioner
{
public:
  static InputParameters validParams();

  PMultigrid(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void setupSolver() override;

  /**
   * Get the coarse levels' function spaces, ascending in order
   * @returns The levels
   */
  const std::vector<std::unique_ptr<Moose::Kokkos::PLevelSpace>> & levels() const
  {
    return _levels;
  }

  /**
   * Get whether each level's operator is to be checked against its diagonal
   * @returns Whether to check
   */
  bool verifyLevelOperators() const { return _verify_level_operators; }

  /**
   * Get whether each level's operator is to be checked against the operator of the next finer level
   * carried through the level transfer
   * @returns Whether to check
   */
  bool verifyLevelGalerkin() const { return _verify_level_galerkin; }

  /**
   * Get whether the assembled operator of each level that assembles one is to be checked against
   * the operator the level applies without a matrix
   * @returns Whether to check
   */
  bool verifyLevelMatrices() const { return _verify_level_matrices; }

  /**
   * Get whether the solver system's own matrix-free operator is to be checked for symmetry
   * @returns Whether to check
   */
  bool verifyOperatorSymmetry() const { return _verify_operator_symmetry; }

protected:
  /// The polynomial order of each coarse level, ascending; the fine level is not listed
  const std::vector<unsigned int> _level_orders;

  /// Whether a smoothed level inverts its entity blocks rather than the operator diagonal
  const bool _entity_block_smoother;

  /// Whether each level's operator is to be checked against its diagonal
  const bool _verify_level_operators;

  /// Whether each level transfer is to be checked for the transpose relationship
  const bool _verify_level_transfers;

  /// Whether each level's operator is to be checked against the operator of the next finer level
  /// carried through the level transfer
  const bool _verify_level_galerkin;

  /// Whether each assembled level operator is to be checked against the matrix-free operator
  const bool _verify_level_matrices;

  /// Whether the solver system's own matrix-free operator is to be checked for symmetry
  const bool _verify_operator_symmetry;

  /// The coarse levels' function spaces, ascending in order
  std::vector<std::unique_ptr<Moose::Kokkos::PLevelSpace>> _levels;
};

#endif

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

  virtual void postLinearization() override;

  virtual void postJacobianAssembly() override;

protected:
  /**
   * Update the operator of every level for the current linearization: a level that carries a matrix
   * reassembles it, and a level that applies its operator as a shell has its PETSc object state
   * bumped instead, so PETSc's PCMG treats the level, and its smoother, as changed.
   */
  void updateLevelOperators();

  ///@{
  /**
   * Checks the 'verify' parameter selects, each a no-op unless it was named. Every one of them
   * costs at least one operator application per degree of freedom, so they are verification aids
   * for small inputs rather than something a production solve carries.
   */
  /// Check each level's operator against the diagonal the level computes
  void verifyLevelOperators();
  /// Check each level's operator against P^T A P, A being the next finer level's operator
  void verifyLevelGalerkin();
  /// Check each assembled level operator against the operator the level applies without a matrix
  void verifyLevelMatrices();
  /// Check each level's entity blocks against its assembled operator
  void verifyEntityBlocks();
  /// Check the solver system's own matrix-free operator against its transpose
  void verifyOperatorSymmetry();
  /// Check the cycle this preconditioner applies against its transpose
  void verifyCycleSymmetry();
  /// Report the norm of the solver system's operator, of its inverse, and their product
  void verifyOperatorConditioning();
  ///@}

  /// The polynomial order of each coarse level, ascending; the fine level is not listed
  const std::vector<unsigned int> _level_orders;

  /// Whether a smoothed level inverts its entity blocks rather than the operator diagonal
  const bool _entity_block_smoother;

  ///@{
  /// Which checks the 'verify' parameter named, read once so that each check site is a named member
  /// rather than a string lookup
  const bool _verify_level_operators;
  const bool _verify_level_transfers;
  const bool _verify_level_galerkin;
  const bool _verify_level_matrices;
  const bool _verify_entity_blocks;
  const bool _verify_cycle_symmetry;
  const bool _verify_operator_conditioning;
  const bool _verify_operator_symmetry;
  ///@}

  /// The coarse levels' function spaces, ascending in order
  std::vector<std::unique_ptr<Moose::Kokkos::PLevelSpace>> _levels;
};

#endif

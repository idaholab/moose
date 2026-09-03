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

#include "libmesh/petsc_matrix.h"

class NonlinearSystemBase;

/**
 * p-multigrid preconditioner ("MG") for the Kokkos matrix-free partial-assembly Jacobian-vector
 * product. Each level named in 'orders' (created by MultigridLevelsAction, ascending, the last
 * entry being the existing fine system) is a separate nonlinear system with its own matrix-free
 * shell operator; level 0 is an ordinary assembled system solved with an algebraic multigrid
 * coarse solve. See idaholab/moose#33644.
 */
class Multigrid : public MoosePreconditioner
{
public:
  static InputParameters validParams();

  Multigrid(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * Assembles level 0's Jacobian and wires PETSc's PCMG onto the fine system's SNES. Called once,
   * from initialSetup(): for linear diffusion (Milestone 1's scope) level 0's operator never
   * changes, so no per-Newton-iteration hook is needed; that is deferred along with nonlinear
   * multigrid.
   */
  void updateLevelOperators();

protected:
  /// Builds the collocation prolongation matrix P_l mapping level (l - 1)'s coefficients to
  /// level l's, for the shared, same-order-family, LAGRANGE-nested case
  std::unique_ptr<libMesh::PetscMatrix<Number>> buildProlongation(unsigned int level) const;

  /// Wires PCMG onto the fine system's SNES/KSP: levels, interpolation operators, per-level
  /// smoothers, and the coarse solve
  void configure();

  /// The polynomial order of each level, ascending; back() is the fine (existing) system's order
  const std::vector<unsigned int> _orders;

  ///@{ Smoother and coarse-solve options (PETSc KSP/PC type strings)
  const std::string _smoother_ksp_type;
  const std::string _smoother_pc_type;
  const unsigned int _smoother_its;
  const std::string _coarse_pc_type;
  ///@}

  /// Get the PETSc Mat backing level l's tagged system matrix (a shell for l > 0, an ordinary
  /// assembled matrix for l == 0)
  Mat levelMat(unsigned int level) const;

  /// The fine variable's name; every level system/variable is named from it
  /// (MultigridLevelsAction::levelSystemName()/levelVariableName())
  const std::string _fine_var_name;

  /// Each level's nonlinear system, ascending order; back() is the fine system (== _nl)
  std::vector<NonlinearSystemBase *> _level_systems;

  /// P_l, mapping level (l - 1) coefficients to level l; _prolongation[l - 1] is P_l for
  /// l = 1 .. orders.size() - 1
  std::vector<std::unique_ptr<libMesh::PetscMatrix<Number>>> _prolongation;

  /// Whether configure() has already wired PCMG onto the fine system's SNES
  bool _configured;
};

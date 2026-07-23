//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMLinearSolverBase.h"
#include "MFEMFESpaceHierarchy.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/fem/multigrid.hpp"
#include "libmesh/restore_warnings.h"

/**
 * P-multigrid / geometric multigrid preconditioner backed by
 * mfem::GeometricMultigrid.
 *
 * The hierarchy is provided by an MFEMFESpaceHierarchy object. The
 * finest-level operator is the constrained operator passed to SetOperator().
 * Coarser levels are rediscretized from the equation system's bilinear integrators.
 *
 * Current implementation:
 *  - Only single-variable equation systems are supported.
 *  - Only linear equation systems are supported.
 *
 * Example input:
 * @code
 * [Solvers]
 *   [boomeramg]
 *     type = MFEMHypreBoomerAMG
 *   []
 *   [pmg]
 *     type = MFEMGeometricMultigridSolver
 *     variable          = u
 *     fespace_hierarchy = h1_hierarchy
 *     smoothers         = 'boomeramg'   # use on all interior levels
 *     coarse_solver     = boomeramg
 *     assembly_levels   = 'legacy'      # use on all levels
 *   []
 *   [main]
 *     type = MFEMCGSolver
 *     preconditioner = pmg
 *   []
 * []
 * @endcode
 */
class MFEMGeometricMultigridSolver : public Moose::MFEM::LinearSolverBase
{
public:
  static InputParameters validParams();

  MFEMGeometricMultigridSolver(const InputParameters & parameters);

  /// Creates a stable proxy solver; the real multigrid is built when the proxy gets an operator.
  void ConstructSolver() override;

protected:
  /// Rebuilds the multigrid hierarchy for the supplied finest-level operator.
  void SetOperatorImpl(mfem::Operator & op) override;

private:
  /// Map assembly-level string ("legacy", "full", "element", "partial", "none")
  /// to the corresponding mfem::AssemblyLevel enum value.
  mfem::AssemblyLevel ParseAssemblyLevel(const std::string & s) const;

  /// Rebuild the multigrid object and per-level operators for the supplied finest-level operator.
  void BuildMultigrid(const mfem::Operator & op);

  /**
   * Stable preconditioner object seen by outer MFEM solvers.
   *
   * MFEM iterative solvers call SetOperator() on their preconditioner, but
   * mfem::GeometricMultigrid::SetOperator() aborts. This proxy keeps a stable
   * solver address for the outer solver and uses SetOperator() as the hook to
   * rebuild the actual multigrid hierarchy.
   */
  class MGProxy : public mfem::Solver
  {
  public:
    /// Constructs a proxy that delegates multigrid rebuilding to the owning MOOSE solver.
    MGProxy(MFEMGeometricMultigridSolver & owner);

    /// Updates the concrete MFEM multigrid object used by Mult().
    void setMG(mfem::GeometricMultigrid & mg);

    /// Rebuilds the owner's multigrid hierarchy for the new outer-solver operator.
    void SetOperator(const mfem::Operator & op) override;

    /// Applies the current concrete MFEM multigrid preconditioner.
    void Mult(const mfem::Vector & x, mfem::Vector & y) const override;

  private:
    /// Solver object that owns the concrete multigrid and level data.
    MFEMGeometricMultigridSolver & _owner;

    /// Non-owning pointer to the currently active concrete MFEM multigrid object.
    mfem::GeometricMultigrid * _mg = nullptr;
  };

  /// Trial variable whose operator is preconditioned by this solver.
  const std::string _var_name;

  /// Finite element space hierarchy defining the multigrid levels.
  std::shared_ptr<mfem::ParFiniteElementSpaceHierarchy> _hierarchy;

  /// Names of solvers used as smoothers on interior multigrid levels.
  const std::vector<MFEMSolverName> _smoother_names;

  /// Name of the solver used on the coarsest multigrid level.
  const MFEMSolverName _coarse_solver_name;

  /// Assembly level requested for each multigrid level after optional single-value expansion.
  std::vector<mfem::AssemblyLevel> _assembly_levels;

  /// Non-owning pointer to the proxy solver stored in _solver.
  MGProxy * _mg_proxy = nullptr;

  /// Rediscretized bilinear forms kept alive for the active linear coarse-level operators.
  std::vector<std::shared_ptr<mfem::ParBilinearForm>> _level_blfs;

  /// Constrained linear coarse-level operators; destroyed before the forms that own their data.
  std::vector<std::unique_ptr<mfem::OperatorHandle>> _level_ops;

  /// Concrete MFEM multigrid preconditioner rebuilt on each SetOperator() call.
  std::unique_ptr<mfem::GeometricMultigrid> _mg;
};

#endif

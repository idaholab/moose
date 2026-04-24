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
 * The hierarchy is provided by a Moose::MFEM::FESpaceHierarchy object. The
 * finest-level operator is the constrained operator passed to SetOperator().
 * Coarser levels are rediscretized from the equation system's bilinear or
 * nonlinear integrators.
 *
 * Limitations (current implementation):
 *  - Only single-variable equation systems are supported.
 *  - Equation systems with active mixed bilinear form contributions are not
 *    supported (see open question #7 in pMG_plan.md).
 *  - For nonlinear equation systems the coarse-level linearization points are
 *    restricted from the most recent fine-level EquationSystem::GetGradient()
 *    point.
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
 *     smoothers         = 'boomeramg'   # broadcast to all interior levels
 *     coarse_solver     = boomeramg
 *     assembly_levels   = 'legacy'      # broadcast to all levels
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

  /// Rebuilds the multigrid hierarchy for the supplied finest-level operator.
  void SetOperator(mfem::Operator & op) override;

private:
  /// Map assembly-level string ("legacy", "full", "element", "partial", "none")
  /// to the corresponding mfem::AssemblyLevel enum value.
  static mfem::AssemblyLevel ParseAssemblyLevel(const std::string & s);

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
    MGProxy(MFEMGeometricMultigridSolver & owner) : _owner(owner) { iterative_mode = false; }

    void setMG(mfem::GeometricMultigrid * mg)
    {
      _mg = mg;
      if (mg)
      {
        height = mg->Height();
        width = mg->Width();
      }
    }

    void SetOperator(const mfem::Operator & op) override { _owner.BuildMultigrid(op); }

    void Mult(const mfem::Vector & x, mfem::Vector & y) const override
    {
      MFEM_VERIFY(_mg, "MGProxy: GeometricMultigrid not yet built");
      _mg->Mult(x, y);
    }

  private:
    MFEMGeometricMultigridSolver & _owner;
    mfem::GeometricMultigrid * _mg = nullptr;
  };

  // ---- parameters (resolved at construction) ----
  const std::string _var_name;
  std::shared_ptr<mfem::ParFiniteElementSpaceHierarchy> _hierarchy;
  const std::vector<MFEMSolverName> _smoother_names; ///< Names of interior-level smoothers
  const MFEMSolverName _coarse_solver_name;
  std::vector<mfem::AssemblyLevel> _assembly_levels; ///< Per-level; broadcast if length 1

  MGProxy * _mg_proxy = nullptr; ///< Non-owning pointer into _solver

  // ---- per-level forms — kept alive across SetOperator calls ----
  std::vector<std::shared_ptr<mfem::ParBilinearForm>> _level_blfs;
  std::vector<std::shared_ptr<mfem::ParNonlinearForm>> _level_nlfs;
  // ---- constrained level operators; must be destroyed before forms ----
  std::vector<std::unique_ptr<mfem::OperatorHandle>> _level_ops;

  // ---- actual GeometricMultigrid (replaced each SetOperator call) ----
  std::unique_ptr<mfem::GeometricMultigrid> _mg;
};

#endif

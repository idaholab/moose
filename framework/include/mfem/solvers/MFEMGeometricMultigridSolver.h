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

namespace Moose::MFEM
{

/**
 * P-multigrid / geometric multigrid preconditioner backed by
 * mfem::GeometricMultigrid.
 *
 * The hierarchy is provided by a Moose::MFEM::FESpaceHierarchy object. The
 * equation system's bilinear (and, if present, nonlinear) integrators are
 * rebuilt at each level via EquationSystem::buildBilinearFormForFESpace /
 * buildNonlinearFormForFESpace each time updateSolver() is called.
 *
 * Limitations (current implementation):
 *  - Only single-variable equation systems are supported.
 *  - Equation systems with active mixed bilinear form contributions are not
 *    supported (see open question #7 in pMG_plan.md).
 *  - For nonlinear equation systems the coarse-level linearization point is
 *    the zero vector (open question #1 in pMG_plan.md).
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
class GeometricMultigridSolver : public LinearSolverBase
{
public:
  static InputParameters validParams();

  GeometricMultigridSolver(const InputParameters & parameters);

  /// Creates a stable proxy solver; the real multigrid is built in updateSolver().
  void constructSolver() override;

  /// Rebuilds the full multigrid hierarchy from the equation system's integrators.
  /// Called once per solve (before the outer linear/nonlinear solver runs).
  void updateSolver(mfem::Operator & op, mfem::Array<int> & tdofs) override;

  /// No-op: operator-level updateSolver handles setup for this solver.
  void setupLOR(mfem::ParBilinearForm & /*a*/, mfem::Array<int> & /*tdofs*/) override {}

private:
  /// Map assembly-level string ("legacy", "full", "element", "partial", "none")
  /// to the corresponding mfem::AssemblyLevel enum value.
  static mfem::AssemblyLevel parseAssemblyLevel(const std::string & s);

  /// Stable proxy that intercepts SetOperator calls from the outer iterative solver
  /// (CG, GMRES, …) and makes them a no-op, preventing the MFEM_ABORT that
  /// mfem::MultigridBase::SetOperator would otherwise trigger.  Its address never
  /// changes, so the outer solver's raw preconditioner pointer stays valid even
  /// after the real mfem::GeometricMultigrid is replaced inside updateSolver().
  class MGProxy : public mfem::Solver
  {
  public:
    MGProxy() { iterative_mode = false; }
    /// Point the proxy at a newly built GeometricMultigrid and sync dimensions.
    void setMG(mfem::GeometricMultigrid * mg)
    {
      _mg = mg;
      if (mg)
      {
        height = mg->Height();
        width = mg->Width();
      }
    }
    /// Absorb the SetOperator call that IterativeSolver propagates to its
    /// preconditioner; just keep height/width consistent.
    void SetOperator(const mfem::Operator & op) override
    {
      height = op.Height();
      width = op.Width();
    }
    void Mult(const mfem::Vector & x, mfem::Vector & y) const override
    {
      MFEM_VERIFY(_mg, "MGProxy: GeometricMultigrid not yet built");
      _mg->Mult(x, y);
    }

  private:
    mfem::GeometricMultigrid * _mg = nullptr; // non-owning; updated each solve
  };

  // ---- parameters (resolved at construction) ----
  const std::string _var_name;
  std::shared_ptr<mfem::ParFiniteElementSpaceHierarchy> _hierarchy;
  const std::vector<std::string> _smoother_names; ///< Names of interior-level smoothers
  const std::string _coarse_solver_name;
  std::vector<mfem::AssemblyLevel> _assembly_levels; ///< Per-level; broadcast if length 1

  // ---- stable proxy (lifetime = GMS lifetime; _solver owns it) ----
  MGProxy * _mg_proxy = nullptr; ///< Non-owning pointer into _solver for easy access

  // ---- actual GeometricMultigrid (replaced each updateSolver call) ----
  std::unique_ptr<mfem::GeometricMultigrid> _mg;

  // ---- per-level forms — kept alive across updateSolver calls ----
  std::vector<std::shared_ptr<mfem::ParBilinearForm>> _level_blfs;
  std::vector<std::shared_ptr<mfem::ParNonlinearForm>> _level_nlfs;
};

} // namespace Moose::MFEM
#endif

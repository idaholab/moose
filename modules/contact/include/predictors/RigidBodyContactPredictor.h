//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Predictor.h"

#include "libmesh/id_types.h"

#include <petscis.h>
#include <petscmat.h>

#include <vector>

/**
 * Predictor that runs a small Newton sub-solve restricted to the contact
 * region (LM DOFs on the contact sideset + displacement DOFs within
 * `k_hops` element hops + optionally the load-control scalar) to
 * warm-start the state before the full monolithic SNES fires.
 *
 * The full-Newton solver is unchanged: this class only modifies the
 * initial guess `sln` that `Transient::takeStep` hands to SNES.  For
 * load-controlled contact where the (scalar_row, scalar_col) Jacobian
 * entry is structurally zero and plain Newton limit-cycles on the
 * coupled (u, lambda, s) block, the sub-solve resolves the contact
 * subproblem to a self-consistent state and the full Newton then only
 * needs a handful of iterations to correct the far-field.  See
 * `uzawa_npc_plan.md` for the design.
 *
 * On sub-solve failure (non-convergence in `sub_max_iter`, KSP
 * breakdown, negative bound projection thrashing) the predictor
 * reverts `sln` to its state at entry and logs a warning: the outer
 * SNES still runs, just from an unmodified initial guess.
 */
class RigidBodyContactPredictor : public Predictor
{
public:
  static InputParameters validParams();
  RigidBodyContactPredictor(const InputParameters & parameters);
  virtual ~RigidBodyContactPredictor();

  virtual void apply(NumericVector<Number> & sln) override;
  /// Extends the base `shouldApply` (repeated-timestep skip, `skip_times`
  /// lists) with a MOOSE-Controls-friendly enable/disable check: when
  /// `enable = false` (settable at runtime by a `Control`) the predictor is
  /// skipped entirely.  See the `enable` param in `MooseObject::validParams`.
  virtual bool shouldApply() override;

private:
  /// Contact sideset(s) -- same as the RigidBodyNodalNCPKernel `boundary`.
  const std::vector<BoundaryName> _boundary_names;
  /// Names of the input variables -- variables do not exist yet when the
  /// Predictor is constructed (predictor action runs before variable
  /// setup), so we defer number lookup to `setupRegion()`.
  const std::string _lm_var_name;
  const std::vector<VariableName> _disp_var_names;
  /// Load-control scalar variable name (empty for displacement control).
  const std::string _scalar_var_name;
  /// Populated in `setupRegion()`.
  unsigned int _lm_var_num;
  std::vector<unsigned int> _disp_var_num;
  /// Number of element hops from the LM sideset to include in the
  /// displacement region.  Larger `k` = larger sub-problem = more
  /// accurate warm-start.  Default 2.
  const unsigned int _k_hops;
  /// Cap on sub-Newton iterations.  Sub-solve exits on convergence,
  /// this cap, or breakdown.
  const unsigned int _sub_max_iter;
  /// Absolute residual tolerance for the sub-solve (on the restricted
  /// residual norm).
  const Real _sub_abs_tol;
  /// Relative residual tolerance (fraction of initial sub-residual).
  const Real _sub_rel_tol;

  /// Global DOF indices in the contact region -- the set the sub-solve
  /// updates.  Sorted, deduplicated, computed once in initialSetup.
  std::vector<libMesh::dof_id_type> _region_dofs;
  /// Subset of `_region_dofs` that are LM DOFs.  Used to enforce
  /// lambda >= 0 by simple projection after each sub-step.
  std::vector<libMesh::dof_id_type> _lm_dofs;
  /// PETSc IS wrapping `_region_dofs` -- used for MatCreateSubMatrix
  /// and VecGetSubVector.  Built in initialSetup, destroyed in dtor.
  IS _region_is;
  /// Sanity flag -- `apply()` bails out immediately if we never built
  /// a non-empty region (e.g. bad boundary name in serial-empty
  /// partition).
  bool _region_ready;
  /// Guard: setup runs lazily on first `apply()` since `Predictor` does
  /// not have a `SetupInterface::initialSetup` hook and we need the
  /// DOF map to exist.
  bool _setup_done;

  /// Compute the region DOFs and build the PETSc IS.  Idempotent; the
  /// first call does the work, later calls no-op.
  void setupRegion();
};

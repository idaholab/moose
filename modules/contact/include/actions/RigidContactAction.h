//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include "libmesh/point.h"

/**
 * Action that expands a single `[RigidContact][<name>]` sub-block into
 * every object the analytic-level-set rigid-contact stack needs:
 * lower-d block, LM variable + bounds, sparsity UO, NCP kernel, one
 * mechanical-contact BC per displacement component, problem-coverage
 * flags, SMP preconditioning, and (when force-controlled) NodalArea
 * UO + scalar variable + RigidBodyLoadControl kernel.
 *
 * The contactor UO itself (`SphereContactor`, `SurfaceMeshContactor`,
 * ...) is NOT created here; users define it in `[UserObjects]` and
 * reference it via the `contactor` param.
 *
 * See `modules/contact/rigid_contact_action_plan.md` for the full
 * design; see the class-description string in `validParams()` for the
 * user-facing summary.
 */
class RigidContactAction : public Action
{
public:
  static InputParameters validParams();
  RigidContactAction(const InputParameters & parameters);

  virtual void act() override;

private:
  /// Helper: the Cartesian axis index (0=x, 1=y, 2=z) matching
  /// `load_direction`.  Throws on non-axis-aligned directions.  Used
  /// to derive the default scalar-variable name for force control
  /// (`indenter_<axis>`).
  unsigned int loadAxisIndex() const;
  /// Default LM variable name (respects any user override).
  std::string lmName() const;
  /// Default lower-d subdomain name (respects any user override).
  std::string lowerDName() const;
  /// Default scalar-variable name for force control.
  std::string scalarName() const;
  /// Default nodal-area aux-variable name.
  std::string nodalAreaName() const;

  /// Per-task builders.  Each is guarded on `_current_task`.
  void addMeshGenerators();
  void addUserObjects();
  void addVariables();
  void addAuxVariables();
  void addBoundsObjects();
  void addNodalKernels();
  void addBCs();
  void addScalarKernels();
  void addProblemFlags();
  void addPreconditioning();
  void addInitialConditions();

  // ---- Cached params ----
  const std::vector<BoundaryName> _boundary;
  const std::vector<VariableName> _displacements;
  const std::string _contactor_name;
  const bool _add_lower_d_block;
  const bool _add_sparsity_uo;
  const bool _enforce_bounds;
  const bool _set_problem_coverage_flags;
  const bool _add_full_smp;
  const Real _c;
  const SubdomainID _lower_d_block_id;
  /// Empty when user didn't supply `force` (displacement-controlled).
  const FunctionName _force;
  const Point _load_direction;
  const Real _kss_stiffness;
  /// Empty when the user supplied a constant `kss_stiffness` (default).
  const FunctionName _kss_stiffness_function;
  /// User-supplied variable-name overrides (may be empty; helpers
  /// provide defaults).
  const std::string _user_lm_name;
  const std::string _user_lower_d_name;
  const std::string _user_scalar_name;
  const std::string _user_nodal_area_name;
  /// Initial value of the load-control scalar (default 0).  Users often
  /// want a small nonzero seed here (e.g. 5e-3 for a sphere sitting at
  /// zero gap over a material top) so the first Newton step has some
  /// LM to bite on.
  const Real _scalar_initial_condition;
};

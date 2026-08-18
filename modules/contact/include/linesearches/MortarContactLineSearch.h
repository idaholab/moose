//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ContactLineSearchBase.h"
#include "MortarContactUtils.h"

#include <optional>
#include <unordered_map>
#include <vector>

class WeightedGapUserObject;
class LMWeightedGapUserObject;
class LMWeightedVelocitiesUserObject;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;

/**
 * Constraint-set-aware line search for mortar mechanical contact. Rather than implementing its
 * own bisection algorithm, this class wraps a standard
 * PETSc backing SNESLineSearch (set up by the base class's setupBackingLineSearch()) and only
 * intervenes when the active contact/stick/slip set changes between the checkpointed iterate and
 * the backing search's proposed iterate:
 *
 * - If the set did not change, the backing search's own result is committed unmodified.
 * - If the set changed but the backing search still achieved a sufficient residual reduction, its
 *   result is committed directly.
 * - Otherwise, a single event-limited step is taken along the Newton direction, bounded by the
 *   predicted step length at which the first active dof's contact/friction switch value would
 *   cross zero (Moose::Mortar::Contact::firstEventGroup()). This deliberately omits the
 *   bound/recovery loop and dense-event escape hatch that a full watchdog would need; those are
 *   left to a future phase.
 * - If neither condition holds and no such event is predicted, the failure is propagated to the
 *   outer SNES unchanged.
 *
 * Event prediction only tracks each dof's normal (open/closed) switch value; it does not predict
 * stick/slip transitions for frictional dofs, since the primary failure mode this phase targets
 * is jamming across the open/closed boundary.
 *
 * The `c`/`normalize_c`/`use_derived_c_normal`/`c_t`/`mu`/`epsilon` parameters must be kept
 * consistent with the matching mortar `[Constraints]` block by the user; this class has no way to
 * automatically cross-check them against the constraints actually assembling the residual.
 */
class MortarContactLineSearch : public ContactLineSearchBase
{
public:
  static InputParameters validParams();

  MortarContactLineSearch(const InputParameters & parameters);

  void initialSetup() override;
  void lineSearch() override;

protected:
  /// Per-dof classification result of a call to classify(), plus the raw normal switch value
  /// (lm_value - c*weighted_gap) needed to predict normal open/closed events (Component B).
  struct Classification
  {
    std::unordered_map<dof_id_type, Moose::Mortar::Contact::ConstraintState> states;
    std::unordered_map<dof_id_type, ADReal> normal_switch_values;
  };

  /**
   * Classify every dof this rank owns in \p _weighted_gap_uo's weighted-gap map, reading
   * Lagrange multiplier and friction Lagrange multiplier values directly from \p solution (via
   * PETSc, not through the system's current solution) so that working-copy vectors never wired
   * into the actual system solution can be classified.
   */
  Classification classify(const Vec & solution) const;

  /// The weighted gap user object providing dofToWeightedGap()
  const WeightedGapUserObject * _weighted_gap_uo = nullptr;

  /// Non-null only when 'use_derived_c_normal' is true; provides per-dof dofToDerivedC() for the
  /// normal switch value
  const LMWeightedGapUserObject * _lm_weighted_gap_uo = nullptr;

  /// Non-null only when 'weighted_velocities_uo' is given; enables frictional classification
  const LMWeightedVelocitiesUserObject * _weighted_velocities_uo = nullptr;

  /// The Lagrange multiplier variable enforcing the normal contact constraint
  const MooseVariable * _lm_var = nullptr;

  /// The frictional Lagrange multiplier variable(s): size 1 in 2D, size 2 (tangential + direction)
  /// in 3D. Empty when friction is not enabled.
  std::vector<const MooseVariable *> _friction_lm_vars;

  /// This factor multiplies the weighted gap; must match the corresponding [Constraints] block's
  /// 'c' parameter
  const Real _c;

  /// Whether to normalize c by the per-dof accumulated mortar weight; must match the
  /// corresponding [Constraints] block
  const bool _normalize_c;

  /// Whether to read the normal stiffness scale per-dof from dofToDerivedC() instead of _c; must
  /// match the corresponding [Constraints] block
  const bool _use_derived_c_normal;

  /// Tangential stiffness scale; must match the corresponding [Constraints] block's 'c_t'
  /// parameter. Only used when friction is enabled
  const Real _c_t;

  /// Constant Coulomb friction coefficient; must match the corresponding [Constraints] block.
  /// Required when 'weighted_velocities_uo' is given
  Real _mu = 0;

  /// Below this raw normal Lagrange multiplier value a dof is classified OPEN regardless of its
  /// frictional switch value; must match the corresponding [Constraints] block's 'epsilon'.
  /// Required when 'weighted_velocities_uo' is given; unused otherwise
  Real _epsilon = 0;

  /// Fractional residual-norm reduction (relative to the checkpointed norm) the backing line
  /// search must achieve for its result to be accepted directly once the contact set has changed
  const Real _direct_accept_tol;

  /// Step-length tolerance used to group near-simultaneous predicted contact/friction switch
  /// events (Moose::Mortar::Contact::firstEventGroup())
  const Real _event_group_tol;

  /// The canonical constraint-state map committed at the end of the previous lineSearch() call,
  /// used only to detect whether the contact set changed since then for ltol loosening
  std::unordered_map<dof_id_type, Moose::Mortar::Contact::ConstraintState> _old_state;

  /// The linear tolerance in effect before this object started loosening it; cached on first use
  /// so it can be restored once the contact set stops changing
  Real _user_ksp_rtol = 0;

  /// Whether _user_ksp_rtol has been cached yet
  bool _user_ksp_rtol_set = false;
};

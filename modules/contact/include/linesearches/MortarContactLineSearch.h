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
 * - Otherwise, a single event-limited step is taken along the Newton direction, overshooting
 *   slightly past the predicted step length at which the first active dof's contact/friction
 *   switch value would cross zero (Moose::Mortar::Contact::firstEventGroup()), then verified to
 *   actually change the set. A residual decrease at that point is committed outright; a residual
 *   increase is committed only under the bounded watchdog exception below.
 * - If neither condition holds and no such event is predicted (even after widening the trial step
 *   up to the full Newton step), the failure is propagated to the outer SNES unchanged.
 *
 * A residual increase from an event-limited step is accepted only while it stays within a factor
 * `watchdog_gamma` of the pre-event iterate's residual, and only until recovery (a residual at or
 * below the pre-event level) is reached within `watchdog_max_iterations` further outer Newton
 * iterations; the watchdog checkpoint is fixed at activation and is not reset by further set
 * changes while it remains active, so successive increases cannot ratchet the allowed excursion
 * upward. If recovery does not happen in time, the solution is rolled back to that checkpoint and
 * the current iteration is reported as failed.
 *
 * Many consecutive event-limited iterations indicate a Newton direction crossing many
 * well-separated switching surfaces one at a time; past `dense_event_threshold` such iterations
 * in a row, the event limit is dropped in favor of a single composite step to the full Newton
 * step, subject to the same verification and watchdog gating.
 *
 * Event prediction only tracks each dof's normal (open/closed) switch value; it does not predict
 * stick/slip transitions for frictional dofs, since the primary failure mode this phase targets
 * is jamming across the open/closed boundary. Acceptance and diagnostics always classify the
 * complete trial iterate, however, so any additional dof transitions coupling produces beyond
 * what was predicted are still detected and, if the applicable decrease or watchdog condition
 * passes, retained as part of the accepted step.
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
  ~MortarContactLineSearch();

  void initialSetup() override;
  void lineSearch() override;

protected:
  /**
   * Whether a candidate with residual norm \p fnorm_candidate may be committed given the
   * watchdog's current state: always true while the watchdog is inactive; otherwise true only if
   * the candidate stays within 'watchdog_gamma' times the checkpointed watchdog residual.
   */
  bool watchdogPermits(Real fnorm_candidate) const;

  /**
   * Activate the watchdog at the just-checkpointed pre-event iterate, caching its solution and
   * residual so every later bound/recovery check in this watchdog episode measures against this
   * SAME original point rather than a rolling one.
   */
  void activateWatchdog(Vec x_ke, Vec f_ke, Real fnorm_ke);

  /**
   * Advance an active watchdog by one outer Newton iteration after committing a candidate with
   * residual norm \p fnorm_committed. Deactivates on recovery. If 'watchdog_max_iterations'
   * iterations pass without recovery, rolls \p X / \p F back to the cached watchdog checkpoint,
   * zeros \p Y, marks \p line_search failed, and deactivates. A no-op while the watchdog is
   * inactive.
   */
  void advanceWatchdog(Real fnorm_committed, Vec X, Vec F, Vec Y, SNESLineSearch line_search);

  /// Tear down the active watchdog's cached checkpoint vectors and reset its bookkeeping.
  void deactivateWatchdog();


  /// Per-dof classification result of a call to classify(), plus the raw switch values
  /// Component B predicts events from: the normal switch value (lm_value - c*weighted_gap,
  /// for OPEN <-> CONTACT events) and, when friction is enabled, the tangential switch value
  /// (radius - ||augmented_tangential_pressure||, for CONTACT_STICK <-> CONTACT_SLIP events).
  /// Stored as plain Real rather than ADReal: this line search only ever evaluates residuals
  /// with AD derivative tracking disabled, so these values carry no usable sensitivities, and
  /// every consumer immediately discards them.
  struct Classification
  {
    std::unordered_map<dof_id_type, Moose::Mortar::Contact::ConstraintState> states;
    std::unordered_map<dof_id_type, Real> normal_switch_values;
    std::unordered_map<dof_id_type, Real> tangential_switch_values;
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

  /// Relative overshoot placing an event-limited trial step just past, rather than exactly at, a
  /// predicted switching event (eq eq:event-step), so the trial point lands unambiguously on the
  /// new branch instead of sitting on the switching surface itself
  const Real _event_step_epsilon;

  /// Number of consecutive outer Newton iterations that have required the event-limited fallback;
  /// reset whenever the backing search's own result is committed (unchanged identity or direct
  /// acceptance), since that indicates the constraint set has stopped forcing single-event steps
  unsigned int _consecutive_event_steps = 0;

  /// Consecutive event-limited iterations after which a single event-by-event step is abandoned
  /// in favor of one composite step to the full Newton step (still subject to the same
  /// verification and watchdog gating), approximating a pathsearch that walks many breakpoints in
  /// one subproblem instead of reassembling once per breakpoint
  const unsigned int _dense_event_threshold;

  /// Bound on how far the residual may exceed the watchdog checkpoint's residual while the
  /// watchdog is active (eq eq:watchdog-bound); must exceed 1
  const Real _watchdog_gamma;

  /// Number of further outer Newton iterations allowed for the residual to recover to at or below
  /// the watchdog checkpoint's level (eq eq:watchdog-recovery) before rolling back
  const unsigned int _watchdog_max_iterations;

  /// Whether a watchdog episode is currently active
  bool _watchdog_active = false;

  /// Outer Newton iterations elapsed since the current watchdog episode was activated
  unsigned int _watchdog_iterations = 0;

  /// Phi(x_ke) = 0.5*||F(x_ke)||^2 at the watchdog checkpoint; every bound/recovery check in the
  /// current episode measures against this fixed value
  Real _watchdog_phi_ke = 0;

  /// Cached solution at the watchdog checkpoint x_ke, restored into the solver's iterate if the
  /// episode times out without recovery; null while no episode is active
  Vec _watchdog_x_ke = nullptr;

  /// Cached residual at the watchdog checkpoint x_ke, restored alongside _watchdog_x_ke on
  /// timeout so the solver's (X, F) pair stays consistent
  Vec _watchdog_f_ke = nullptr;
};

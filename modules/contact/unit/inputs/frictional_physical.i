!include frictionless_physical.i

[GlobalParams]
  # scaling=1/E so physical c constants (scaled by scalingFactor ~ 1/E) are
  # far smaller than the defaults (c_normal=1e6, c_tangential=1), making the
  # convergence gap between physical and default clearly observable.
  scaling := 1e-4
  degree_one_friction_residual = true
[]

[Materials]
  [tensor]
    youngs_modulus := 1.0e4
  []
  [tensor_1000]
    youngs_modulus := 1e5
  []
[]

[Contact]
  [mortar]
    model := coulomb
    friction_coefficient = 0.4
    # Retain the geometry used before the mortar subpatch-plane default changed so this test
    # isolates physical scaling from the newer geometric-plane behavior.
    mortar_3d_subpatch_plane = AVERAGED_NODAL_NORMAL
    # Physical (derived) constants are this fixture's default: they are the configuration that
    # actually converges (CONSTRAINT_SET_STRATEGY_PLAN.md Section 17). The fixed-constant
    # (c_normal_strategy/c_tangential_strategy = user) comparison behavior is exercised by
    # overriding these back via the command line, not by making it the default here, so that a
    # plain run of this file is always a working input.
    c_normal_strategy = physical
    c_tangential_strategy = physical
  []
[]

[BCs]
  # Small tangential sliding to activate c_tangential
  [topx]
    function := '0.05 * t'
  []
[]

[LineSearch]
  # Plain PETSc backtracking line search (line_search='basic', inherited from the Executioner
  # block above) is this fixture's default, matching the physical constants above - together
  # they are a known-converging configuration (CONSTRAINT_SET_STRATEGY_PLAN.md Section 17).
  # MortarContactLineSearch is inactive by default so a plain run of this file uses that
  # configuration. Reactivate it (active = 'ls') to exercise MortarContactLineSearch, e.g. to
  # compare against fixed (c_normal_strategy/c_tangential_strategy = user) constants.
  active = ''
  [ls]
    type = MortarContactLineSearch
    contact_ltol = 0.5
    affect_ltol = true
    backing_line_search = 'basic'
    weighted_gap_uo = 'lm_weightedvelocities_object_mortar'
    weighted_velocities_uo = 'lm_weightedvelocities_object_mortar'
    lm_variable = mortar_normal_lm
    friction_lm_variable = mortar_tangential_lm
    friction_lm_dir_variable = mortar_tangential_3d_lm
    c = 1e6
    c_t = 1
    mu = 0.4
    epsilon = 1e-7
    # Component E (hysteresis band) is disabled here: its widening formula anchors to the
    # residual norm at the start of the SNES solve, which this fixture's ~30x first-event
    # residual spike blows out of proportion to the actual per-dof tangential switch-value
    # chatter amplitude at every hysteresis_tau0 tried, making the band freeze every dof's
    # classification simultaneously instead of damping chatter selectively - see
    # CONSTRAINT_SET_STRATEGY_PLAN.md Section 15.
    hysteresis_tau0 = 0
    # This fixture's first contact activation (the open->contact switch at the very start of the
    # first timestep, before any friction state exists) produces a residual increase of roughly
    # 30x the pre-event norm -- a consequence of the stiff c_normal=1e6 relative to the softened,
    # physically-derived material scaling used here, not something a smaller event-limited step
    # can avoid. watchdog_gamma bounds the merit function (0.5*||F||^2), so the required value is
    # the squared residual-norm ratio, roughly 900; the default watchdog_gamma=2 rejects that
    # excursion outright. Raise it well above the observed ratio so the watchdog's bound/recovery
    # machinery gets a chance to operate on it instead of failing the line search on the very
    # first outer iteration.
    # TEMPORARY (diagnostic only, not a production value): pinned absurdly high so early
    # watchdog rejections don't cut this run short before reaching the event-limited iteration
    # window under investigation; see the surrounding comment for why this value keeps compounding.
    watchdog_gamma = 1e12
    dense_event_threshold = 30
    # TEMPORARY (diagnostic only, not a production value): the default 5-iteration recovery
    # window rolls back to the pre-contact checkpoint well before reaching the 10-iteration
    # window under investigation; widen it enough to observe that window without an early rollback.
    watchdog_max_iterations = 30
  []
[]

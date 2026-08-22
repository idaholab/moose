starting_point = 0.04
offset = 0.00

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
  # scaling=1/E so physical c constants (scaled by scalingFactor ~ 1/E) are
  # far smaller than the defaults (c_normal=1e6, c_tangential=1), making the
  # convergence gap between physical and default clearly observable.
  scaling = 1e-4
  degree_one_friction_residual = true
[]

[Mesh]
  [top_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmin = -0.25
    xmax = 0.25
    ymin = -0.25
    ymax = 0.25
    zmin = -0.25
    zmax = 0.25
    elem_type = HEX8
  []
  [rotate_top_block]
    type = TransformGenerator
    input = top_block
    transform = ROTATE
    vector_value = '0 0 0'
  []
  [top_block_sidesets]
    type = RenameBoundaryGenerator
    input = rotate_top_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'top_bottom top_back top_right top_front top_left top_top'
  []
  [top_block_id]
    type = SubdomainIDGenerator
    input = top_block_sidesets
    subdomain_id = 1
  []
  [bottom_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 2
    xmin = -.5
    xmax = .5
    ymin = -.5
    ymax = .5
    zmin = -.3
    zmax = -.25
    elem_type = HEX8
  []
  [bottom_block_id]
    type = SubdomainIDGenerator
    input = bottom_block
    subdomain_id = 2
  []
  [bottom_block_change_boundary_id]
    type = RenameBoundaryGenerator
    input = bottom_block_id
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'top_block_id bottom_block_change_boundary_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'top_block bottom_block'
  []
  [bottom_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = block_rename
    new_boundary = bottom_right
    block = bottom_block
    normal = '1 0 0'
  []
  [bottom_left_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_right_sideset
    new_boundary = bottom_left
    block = bottom_block
    normal = '-1 0 0'
  []
  [bottom_top_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_left_sideset
    new_boundary = bottom_top
    block = bottom_block
    normal = '0 0 1'
  []
  [bottom_bottom_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_top_sideset
    new_boundary = bottom_bottom
    block = bottom_block
    normal = '0  0 -1'
  []
  [bottom_front_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_bottom_sideset
    new_boundary = bottom_front
    block = bottom_block
    normal = '0 1 0'
  []
  [bottom_back_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_front_sideset
    new_boundary = bottom_back
    block = bottom_block
    normal = '0 -1 0'
  []
  allow_renumbering = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = false
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e4
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [tensor_1000]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e5
    poissons_ratio = 0.0
  []
  [stress_1000]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  []
[]

[Contact]
  [mortar]
    primary = 'bottom_top'
    secondary = 'top_bottom'
    formulation = mortar
    model = coulomb
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
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  [botz]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  # Small tangential sliding to activate c_tangential
  [topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top_top'
    function = '0.05 * t'
  []
  [topy]
    type = DirichletBC
    variable = disp_y
    boundary = 'top_top'
    value = 0.0
  []
  [topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'top_top'
    function = '-${starting_point} * t / 0.125 + ${offset}'
  []
[]

[Executioner]
  type = Transient
  end_time = .125
  dt = .025
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
  petsc_options_value = 'lu       mumps                      NONZERO'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
  # Plain PETSc backtracking line search is this fixture's default, matching the physical
  # constants above - together they are a known-converging configuration
  # (CONSTRAINT_SET_STRATEGY_PLAN.md Section 17). Ignored whenever the [LineSearch][ls] block
  # below is activated (a MOOSE LineSearch object, not a PETSc-native one, supersedes this).
  line_search = 'basic'
[]

[LineSearch]
  # MortarContactLineSearch is inactive by default so a plain run of this file uses the
  # known-converging basic-line-search/physical-constants configuration above. Reactivate it
  # (active = 'ls') to exercise MortarContactLineSearch, e.g. to compare against fixed
  # (c_normal_strategy/c_tangential_strategy = user) constants.
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'num_nl cumulative contact'
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [contact]
    type = ContactDOFSetSize
    variable = mortar_normal_lm
    subdomain = 'mortar_secondary_subdomain'
    execute_on = 'nonlinear timestep_end'
  []
[]

[Outputs]
[]

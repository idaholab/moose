# Two blocks bonded by a mortar cohesive zone are peeled apart from x = 0, so the debonded region
# grows from left to right. The lower (secondary) block has 4 interface faces and the upper
# (primary) block 3, so each secondary face overlaps one or two primary faces.
#
# As each secondary face fully separates, it must be added to czm_pressure_secondary, and every
# primary face it overlapped in the undisplaced configuration must be added to
# czm_pressure_primary. The length and first moment (integral of x) of each sideset record which
# faces it holds: a sideset holding the first k faces of width h from x = 0 has length k h and
# first moment (k h)^2 / 2. Once the whole interface has failed the blocks carry no load, so the
# void between the two sidesets must equal the integral of the prescribed opening,
# opening * t * (1 - x / 2), over x in [0, 1], which is 3 / 4 * opening * t.

opening = 0.1

[Mesh]
  # The interface lies on y = 0 and the upper block is not displaced in x, so the void integrand
  # -y n_y vanishes on the vertical closing segments at both ends of the void, which belong to
  # neither sideset.
  [lower]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = -0.5
    ymax = 0
    nx = 4
    ny = 1
    boundary_name_prefix = lower
  []
  [lower_id]
    type = SubdomainIDGenerator
    input = lower
    subdomain_id = 1
  []
  [upper]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 0
    ymax = 0.5
    nx = 3
    ny = 1
    boundary_name_prefix = upper
    boundary_id_offset = 10
  []
  [upper_id]
    type = SubdomainIDGenerator
    input = upper
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'lower_id upper_id'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = secondary_lower
    sidesets = lower_top
    input = combined
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    new_block_name = primary_lower
    sidesets = upper_bottom
    input = secondary
  []

  add_sideset_names = 'czm_pressure_primary czm_pressure_secondary'
  add_sideset_ids = '1000 1001'
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = SMALL
  add_variables = true
  use_automatic_differentiation = true
  block = '1 2'
[]

[AuxVariables]
  [damage]
  []
[]

[AuxKernels]
  [damage]
    type = CohesiveZoneMortarUserObjectAux
    variable = damage
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_damage
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    boundary = lower_bottom
    variable = disp_x
    value = 0
  []
  [fix_y]
    type = DirichletBC
    boundary = lower_bottom
    variable = disp_y
    value = 0
  []
  [fix_upper_x]
    type = DirichletBC
    boundary = upper_top
    variable = disp_x
    value = 0
  []
  [peel_y]
    type = FunctionDirichletBC
    boundary = upper_top
    variable = disp_y
    function = '${opening} * t * (1 - x / 2)'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    poissons_ratio = 0
    block = '1 2'
  []
  [stress]
    type = ADComputeLinearElasticStress
    block = '1 2'
  []
  [normal_strength]
    type = GenericConstantMaterial
    prop_names = normal_strength
    prop_values = 1e4
  []
  [shear_strength]
    type = GenericConstantMaterial
    prop_names = shear_strength
    prop_values = 1e4
  []
[]

[UserObjects]
  [czm_uo]
    type = BilinearMixedModeCohesiveZoneModel
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    disp_x = disp_x
    disp_y = disp_y
    secondary_variable = disp_x
    use_physical_gap = true
    correct_edge_dropping = true
    normal_strength = normal_strength
    shear_strength = shear_strength
    penalty_stiffness = 1e12
    power_law_parameter = 2
    # Fully separated (damage = 1) once the opening exceeds 2 GI_c / normal_strength = 4e-2, which
    # is much larger than the 1e-5 elastic stretch of the blocks at peak traction, so the response
    # does not snap back
    GI_c = 200
    GII_c = 200
  []
[]

[Constraints]
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
  []
  [c_x]
    type = MortarGenericTraction
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
  [c_y]
    type = MortarGenericTraction
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
[]

[MeshModifiers]
  [pressure_sideset_updater]
    type = CZMPressureSidesetUpdater
    block = secondary_lower
    damage_variable = damage
    cohesive_zone_userobject = czm_uo
    primary_boundary = upper_bottom
    secondary_boundary = lower_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    pressure_boundary_primary = czm_pressure_primary
    pressure_boundary_secondary = czm_pressure_secondary
    execute_on = TIMESTEP_END
    # Run before the default group so that the postprocessors see faces promoted this step
    execution_order_group = -1
  []
[]

[Postprocessors]
  # Undisplaced length and first moment of each sideset
  [primary_length]
    type = AreaPostprocessor
    boundary = czm_pressure_primary
  []
  [secondary_length]
    type = AreaPostprocessor
    boundary = czm_pressure_secondary
  []
  [primary_moment]
    type = FunctionSideIntegral
    boundary = czm_pressure_primary
    function = x
  []
  [secondary_moment]
    type = FunctionSideIntegral
    boundary = czm_pressure_secondary
    function = x
  []
  # Area between the two sidesets in the current configuration
  [void_area]
    type = InternalVolume
    boundary = 'czm_pressure_primary czm_pressure_secondary'
    component = 1
  []
  [expected_void_area_separated]
    type = ParsedPostprocessor
    expression = '0.75 * ${opening} * t'
    use_t = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  # Small enough that each secondary face separates in its own time step (t = 0.45, 0.55, 0.6,
  # 0.7), so that every intermediate state of both sidesets is checked
  dt = 0.05
  end_time = 0.8
[]

[Outputs]
  csv = true
[]

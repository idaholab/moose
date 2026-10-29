# Pressurization of the sidesets built by CZMPressureSidesetUpdater. The
# czm_pressure_sideset.i test is repeated here while the  upper block also
# slides tangentially, so the faces paired in the undisplaced configuration are no
# longer aligned. Once the whole interface has failed the blocks carry no load, so the void between
# the two sidesets must equal the integral of the prescribed opening, opening * t * (1 - x / 2),
# over x in [0, 1], which is 3 / 4 * opening * t.
#
# The void is a cavity, connected to an unmeshed plenum of volume Vp, whose gas pressure follows
# the ideal gas law, p (V + Vp) = n R T, and acts on both sidesets, which is only possible if the
# pressure boundary condition picks up the faces added during the run. Gas is injected at a constant
# rate from full separation at t = 0.8 until t = 1, so the pressure first rises, and then falls as
# the peel keeps enlarging the void. Each block is in uniaxial compression, sigma_yy = -p, so that
#   - the vertical reactions on lower_bottom and upper_top are +p and -p times the interface length;
#   - the lower block displaces by u_y = -p / E * (y + 0.5), and the upper block by
#     p / E * (0.5 - y) in addition to the imposed peel;
#   - the void area is V = V0 + p / E, with V0 = 3 / 4 * opening * t, as each block of height 0.5
#     shortens by p / (2 E).
# These fields are linear, so the elements reproduce them exactly. Substituting V into the gas law
# gives p^2 / E + W p - n R T = 0 with W = V0 + Vp, whose positive root is
#   p = E / 2 * (sqrt(W^2 + 4 n R T / E) - W).

opening = 0.1
slide = 0.02
youngs_modulus = 1e9
gas_constant = 8.314
gas_temperature = 300
# Plenum volume, of the order of the void area, which keeps the gas volume positive while the void
# is still closed
plenum_volume = 0.05
# Injection rate chosen so that the pressure peaks near 1e7 at t = 1; p / E = 0.01 is large enough
# to see in the displacements
gas_rate = 2700

[Mesh]
  # The interface lies on y = 0, so the void integrand -y n_y vanishes on the vertical closing
  # segment at the crack front, which belongs to neither sideset.
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

[Problem]
  extra_tag_vectors = 'ref'
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = SMALL
  add_variables = true
  use_automatic_differentiation = true
  extra_vector_tags = 'ref'
  block = '1 2'
[]

[AuxVariables]
  [damage]
  []
  [react_y]
    block = '1 2'
  []
[]

[AuxKernels]
  [damage]
    type = CohesiveZoneMortarUserObjectAux
    variable = damage
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_damage
  []
  [react_y]
    type = ReactionForceAux
    vector_tag = 'ref'
    v = disp_y
    variable = react_y
    block = '1 2'
  []
[]

[Functions]
  [gas_moles]
    type = ParsedFunction
    expression = '${gas_rate} * max(min(t, 1) - 0.8, 0)'
  []
  [expected_pressure]
    type = ParsedFunction
    expression = 'W := 0.75 * ${opening} * t + ${plenum_volume};
                  n := ${gas_rate} * max(min(t, 1) - 0.8, 0);
                  ${youngs_modulus} / 2 * (sqrt(W^2 + 4 * n * ${gas_constant} * ${gas_temperature} / ${youngs_modulus}) - W)'
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
  [slide_x]
    type = FunctionDirichletBC
    boundary = upper_top
    variable = disp_x
    function = '${slide} * t'
  []
  [peel_y]
    type = FunctionDirichletBC
    boundary = upper_top
    variable = disp_y
    function = '${opening} * t * (1 - x / 2)'
  []
  [CavityPressure]
    [void]
      boundary = 'czm_pressure_primary czm_pressure_secondary'
      R = ${gas_constant}
      temperature = gas_temperature
      volume = void_area
      material_input = gas_moles
      additional_volumes = plenum_volume
      temperature_of_additional_volumes = gas_temperature
      output = cavity_pressure
      use_automatic_differentiation = true
      # Act on the undisplaced faces so that the load stays purely normal to the interface and the
      # solution above is exact
      use_displaced_mesh = false
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = ${youngs_modulus}
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
  # Area between the two sidesets in the current configuration, which is the cavity volume
  [void_area]
    type = InternalVolume
    boundary = 'czm_pressure_primary czm_pressure_secondary'
    component = 1
    execute_on = 'initial linear timestep_end'
  []
  [expected_void_area_separated]
    type = ParsedPostprocessor
    expression = '0.75 * ${opening} * t + expected_pressure / ${youngs_modulus}'
    pp_names = 'expected_pressure'
    use_t = true
  []
  # Cavity inputs
  [gas_moles]
    type = FunctionValuePostprocessor
    function = gas_moles
    execute_on = 'initial linear timestep_end'
  []
  [gas_temperature]
    type = ConstantPostprocessor
    value = ${gas_temperature}
    execute_on = 'initial linear timestep_end'
  []
  [plenum_volume]
    type = ConstantPostprocessor
    value = ${plenum_volume}
    execute_on = 'initial linear timestep_end'
  []
  # Pressure checks, valid after full separation
  [lower_reaction_y]
    type = NodalSum
    variable = react_y
    boundary = lower_bottom
  []
  [upper_reaction_y]
    type = NodalSum
    variable = react_y
    boundary = upper_top
  []
  [expected_pressure]
    type = FunctionValuePostprocessor
    function = expected_pressure
  []
  # The lower block moves only because of the pressure
  [lower_disp_y]
    type = PointValue
    variable = disp_y
    point = '0.5 -0.25 0'
  []
  [expected_lower_disp_y]
    type = ParsedPostprocessor
    expression = '-expected_pressure / ${youngs_modulus} * 0.25'
    pp_names = 'expected_pressure'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 0.1
  end_time = 1.2
[]

[Outputs]
  csv = true
[]

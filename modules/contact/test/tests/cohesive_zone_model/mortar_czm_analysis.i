[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 5
    ny = 5
    boundary_name_prefix = bottom
  []
  [msh_id]
    type = SubdomainIDGenerator
    input = msh
    subdomain_id = 1
  []
  [msh_two]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 1
    ymax = 2
    nx = 5
    ny = 5
    boundary_name_prefix = top
    boundary_id_offset = 10
  []
  [msh_two_id]
    type = SubdomainIDGenerator
    input = msh_two
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'msh_id msh_two_id'
  []
  [top_node]
    type = ExtraNodesetGenerator
    coord = '0 2 0'
    input = combined
    new_boundary = top_node
  []
  [bottom_node]
    type = ExtraNodesetGenerator
    coord = '0 0 0'
    input = top_node
    new_boundary = bottom_node
  []
  # Build subdomains
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    sidesets = 'bottom_top'
    input = bottom_node
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    sidesets = 'top_bottom'
    new_block_name = 'primary_lower'
    input = secondary
  []
  allow_renumbering = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      generate_output = 'stress_yy'
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
        block = '1 2'
      []
    []
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []
  [lateral_top]
    type = FunctionDirichletBC
    boundary = top_top
    variable = disp_x
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),0.6-t))'
    preset = true
  []
  [top]
    type = FunctionDirichletBC
    boundary = top_top
    variable = disp_y
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),0.6-t))'
    preset = true
  []
  [bottom]
    type = DirichletBC
    boundary = bottom_bottom
    variable = disp_y
    value = 0
    preset = true
  []
[]

[AuxVariables]
  [mode_mixity_ratio]
  []
  [damage]
  []
  [local_normal_jump]
  []
  [local_tangential_jump]
  []
[]

[AuxKernels]
  [mode_mixity_ratio]
    type = CohesiveZoneMortarUserObjectAux
    variable = mode_mixity_ratio
    user_object = czm_uo
    cohesive_zone_quantity = mode_mixity_ratio
    boundary = 'bottom_top'
  []
  [cohesive_damage]
    type = CohesiveZoneMortarUserObjectAux
    variable = damage
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_damage
    boundary = 'bottom_top'
  []
  [local_normal_jump]
    type = CohesiveZoneMortarUserObjectAux
    variable = local_normal_jump
    user_object = czm_uo
    cohesive_zone_quantity = local_normal_jump
    boundary = 'bottom_top'
  []
  [local_tangential_jump]
    type = CohesiveZoneMortarUserObjectAux
    variable = local_tangential_jump
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump
    boundary = 'bottom_top'
  []
[]

[Materials]
  [stress]
    type = ADComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
    block = '1 2'
  []
  [normal_strength]
    type = GenericFunctionMaterial
    prop_names = 'N'
    prop_values = 'if(x<0.5,1,100)*1e4'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 30
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  start_time = 0.0
  dt = 0.0005
  end_time = 0.01
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [czm_uo]
    type = BilinearMixedModeCohesiveZoneModel
    primary_boundary = 'top_bottom'
    secondary_boundary = 'bottom_top'
    primary_subdomain = 10000
    secondary_subdomain = 10001

    disp_x = disp_x
    disp_y = disp_y
    friction_coefficient = 0.1 # with 2.0 works
    secondary_variable = disp_x

    penalty = 0e6
    penalty_friction = 1e4
    use_physical_gap = true

    correct_edge_dropping = true
    normal_strength = N
    shear_strength = 1e3
    viscosity = 1e-3
    penalty_stiffness = 1e6
    mixed_mode_criterion = POWER_LAW

    power_law_parameter = 2.2
    GI_c = 1e3
    GII_c = 1e2
    displacements = 'disp_x disp_y'
  []
[]

[Constraints]
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = 'top_bottom'
    secondary_boundary = 'bottom_top'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
    correct_edge_dropping = true
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = 'top_bottom'
    secondary_boundary = 'bottom_top'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
    correct_edge_dropping = true
  []
  [c_x]
    type = MortarGenericTraction
    primary_boundary = 'top_bottom'
    secondary_boundary = 'bottom_top'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
    correct_edge_dropping = true
  []
  [c_y]
    type = MortarGenericTraction
    primary_boundary = 'top_bottom'
    secondary_boundary = 'bottom_top'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
    correct_edge_dropping = true
  []
[]

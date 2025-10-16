[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1.1
    ymax = 1
    xmin = -0.1
    nx = 5
    ny = 5
  []
  [rename_base]
    type = RenameBoundaryGenerator
    input = base
    old_boundary = 'top bottom left right'
    new_boundary = 'top_base bottom_base left_base right_base'
  []
  [base_id]
    type = SubdomainIDGenerator
    input = rename_base
    subdomain_id = 1
  []

  [top]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 1
    ymax = 2
    nx = 10
    ny = 10
  []
  [rename_top]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = 'top bottom left right'
    new_boundary = '100 101 102 103'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = rename_top
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'base_id top_id'
  []
  [top_node]
    type = ExtraNodesetGenerator
    coord = '0 2 0'
    input = combined
    new_boundary = top_node
  []
  [bottom_node]
    type = ExtraNodesetGenerator
    coord = '-0.1 0 0'
    input = top_node
    new_boundary = bottom_node
  []

  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    sidesets = 'top_base'
    input = bottom_node
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    sidesets = '101'
    new_block_name = 'primary_lower'
    input = secondary
  []

  patch_update_strategy = auto
  patch_size = 20
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
        decomposition_method = EigenSolution
        generate_output = 'vonmises_stress'
        block = '1 2'
      []
    []
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    preset = false
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []

  [fix_top]
    type = FunctionDirichletBC
    preset = false
    boundary = 100
    variable = disp_x
    function = 'if(t<1.0,0,t-1.0)'
  []

  [top]
    type = FunctionDirichletBC
    boundary = 100
    variable = disp_y
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),if(t<0.8,0.6-t,-0.2)))'
    preset = false
  []

  [bottom]
    type = DirichletBC
    boundary = bottom_base
    variable = disp_y
    value = 0
    preset = false
  []
[]

[Materials]
  [normal_strength]
    type = GenericConstantMaterial
    prop_names = 'normal_strength'
    prop_values = '1e3'
  []
  [shear_strength]
    type = GenericConstantMaterial
    prop_names = 'shear_strength'
    prop_values = '7.5e2'
  []
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
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  line_search = 'none'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true
  compute_scaling_once = false
  off_diagonals_in_auto_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  start_time = 0.0
  dt = 0.1
  end_time = 2
  #dtmin = 0.1
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [czm_uo]
    type = BilinearMixedModeCohesiveZoneModel
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001

    correct_edge_dropping = true

    disp_x = disp_x
    disp_y = disp_y
    secondary_variable = disp_x

    use_physical_gap = true

    apply_zero_traction_after_damage = false

    # bilinear model parameters
    normal_strength = 'normal_strength'
    shear_strength = 'shear_strength'
    penalty_stiffness = 1e8
    power_law_parameter = 2
    GI_c = 123
    GII_c = 54
    displacements = 'disp_x disp_y'
  []
  # [mortar_uo]
  #   type = PenaltyWeightedGapUserObject
  #   primary_boundary = 101
  #   secondary_boundary = 'top_base'
  #   primary_subdomain = 10000
  #   secondary_subdomain = 10001
  #   disp_x = disp_x
  #   disp_y = disp_y
  #   penalty = 1e8
  #   use_physical_gap = true
  # []
  # [mortar_uo]
  #   type = PenaltyFrictionUserObject
  #   primary_boundary = 101
  #   secondary_boundary = 'top_base'
  #   primary_subdomain = 10000
  #   secondary_subdomain = 10001
  #   disp_x = disp_x
  #   disp_y = disp_y
  #   friction_coefficient = 0.4
  #   secondary_variable = disp_x
  #   penalty = 1e8
  #   penalty_friction = 5e6
  # []
[]

[Constraints]
  [c_x]
    type = MortarGenericTraction
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
  [c_y]
    type = MortarGenericTraction
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
[]

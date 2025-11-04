[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1.0
    ymax = 1
    xmin = -0.0
    nx = 1
    ny = 1
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
    nx = 1
    ny = 1
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
    coord = '-0.0 0 0'
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
      generate_output = 'stress_yy vonmises_stress stress_xy'
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
        generate_output = 'vonmises_stress stress_yy stress_xy'
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
  [fix_top]
    type = DirichletBC
    preset = true
    boundary = 100
    variable = disp_x
    value = 0
  []
  [top]
    type = FunctionDirichletBC
    boundary = 100
    variable = disp_y
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),0.6-t))'
    preset = true
  []
  [bottom]
    type = DirichletBC
    boundary = bottom_base
    variable = disp_y
    value = 0
    preset = true
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
[]

[Postprocessors]
  [stress_yy]
    type = ElementExtremeValue
    variable = stress_yy
    value_type = max
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

#  petsc_options = '-pc_svd_monitor -ksp_monitor_singular_values'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'svd        superlu_dist'

  automatic_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 150
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-12
  start_time = 0.0
  dt = 0.01
  end_time = 0.85
  dtmin = 0.01
[]

[Outputs]
  exodus = true
  csv = true
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
    friction_coefficient = 0.0 # with 2.0 works
    secondary_variable = disp_x
    penalty = 0e6
    penalty_friction = 0e4
    use_physical_gap = true

    # bilinear parameters
    normal_strength = 1e4
    shear_strength = 1e3
    penalty_stiffness = 1e6
    power_law_parameter = 2.2
    viscosity = 1.0e-3
    GI_c = 1e3
    GII_c = 1e2
    displacements = 'disp_x disp_y'
  []
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

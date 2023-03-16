[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = iron.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    sidesets = '10'
    input = input_file
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    sidesets = '20'
    new_block_name = 'primary_lower'
    input = secondary
  []
  patch_update_strategy = auto
  patch_size = 20
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [frictionless_normal_lm]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
    use_dual = true
  []
  [tangential_lm]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
    use_dual = true
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [saved_x]
  []
  [saved_y]
  []
  [diag_saved_x]
  []
  [diag_saved_y]
  []
  [von_mises]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 2. 8.'
    y = '0. -1.0 -1.0'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 8.' # x = '0. 2. 8.'
    y = '0. 8.' # y = '0. 0. 8'
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    block = '1 2'
    strain = FINITE
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
    block = '1 2'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
    block = '1 2'
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
    block = '1 2'
  []
  [von_mises_kernel]
    #Calculates the von mises stress and assigns it to von_mises
    type = RankTwoScalarAux
    variable = von_mises
    rank_two_tensor = stress
    execute_on = timestep_end
    scalar_type = VonMisesStress
    block = '1 2'
  []
[]

[Postprocessors]
  [bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 20
  []
  [bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 20
  []
  [top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 10
  []
  [top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 10
  []
  [_dt]
    type = TimestepSize
  []
  [contact_pressure]
    type = NodalVariableValue
    variable = frictionless_normal_lm
    nodeid = 805
  []
[]

[BCs]
  [bot_x_disp]
    type = DirichletBC
    variable = disp_x
    boundary = '40'
    value = 0.0
    preset = false
  []
  [bot_y_disp]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
    preset = false
  []
  [top_y_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '30'
    function = disp_ramp_vert
    preset = false
  []
  [top_x_disp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '30'
    function = disp_ramp_horz
    preset = false
  []
[]

[Materials]
  [stuff1_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 6896
    poissons_ratio = 0.32
  []
  [stuff1_strain]
    type = ComputeFiniteStrain
    block = '2'
  []
  [stuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  []
  [stuff2_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 689.6
    poissons_ratio = 0.32
  []
  [stuff2_strain]
    type = ComputeFiniteStrain
    block = '1'
  []
  [stuff2_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'
  line_search = 'none'

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7
  l_tol = 1e-6

  l_max_its = 50
  nl_max_its = 30

  start_time = 0.0
  end_time = 0.1 # 6.5

  dt = 0.0125
  dtmin = 1e-5
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [cont_press]
    type = NodalValueSampler
    variable = frictionless_normal_lm
    boundary = '10'
    sort_by = id
    execute_on = FINAL
  []
  [friction]
    type = NodalValueSampler
    variable = tangential_lm
    boundary = '10'
    sort_by = id
    execute_on = FINAL
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = false
  csv = true

  [chkfile]
    type = CSV
    show = 'cont_press friction'
    start_time = 0.0
    execute_vector_postprocessors_on = FINAL
  []

  [console]
    type = Console
    max_rows = 5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    lm_variable_normal = frictionless_normal_lm
    lm_variable_tangential_one = tangential_lm
    secondary_variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
  []
[]

[Constraints]
  # All constraints below for mechanical contact (Mortar)
  [weighted_gap_lm]
    type = ComputeFrictionalForceLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    variable = frictionless_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    friction_lm = tangential_lm
    mu = 0.5
    c_t = 1.0e1
    c = 1.0e3
    weighted_gap_uo = weighted_vel_uo
    weighted_velocities_uo = weighted_vel_uo
  []
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = frictionless_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_vel_uo
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = frictionless_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_vel_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = tangential_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
[]

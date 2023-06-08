[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = hertz_cyl_coarser.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    sidesets = '3'
    input = input_file
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    sidesets = '2'
    new_block_name = 'primary_lower'
    input = secondary
  []
  allow_renumbering = false
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [frictionless_normal_lm]
    block = 'secondary_lower'
    use_dual = true
  []
  [tangential_lm]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
[]

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. -0.020 -0.020'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. 0.0 0.015'
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  save_in = 'saved_x saved_y'
  extra_vector_tags = 'ref'
  block = '1 2 3 4 5 6 7'
  generate_output = 'stress_xx stress_yy stress_xy'
[]

[Postprocessors]
  [bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  []
  [bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  []
  [top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 4
  []
  [top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 4
  []
  [_dt]
    type = TimestepSize
  []
  [num_lin_it]
    type = NumLinearIterations
  []
  [num_nonlin_it]
    type = NumNonlinearIterations
  []
[]

[BCs]
  [side_x]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2'
    value = 0.0
  []
  [bot_y]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  []
  [top_y_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = disp_ramp_vert
  []
  [top_x_disp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = disp_ramp_horz
  []
[]

[Materials]
  [stuff1_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e10
    poissons_ratio = 0.0
  []
  [stuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [stuff2_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2 3 4 5 6 7'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stuff2_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2 3 4 5 6 7'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-15                   1e-5'

  line_search = 'none'

  nl_abs_tol = 1e-7

  start_time = 0.0
  end_time = 0.3 # 3.5
  l_tol = 1e-4
  dt = 0.1
  dtmin = 0.001
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [surface]
    type = NodalValueSampler
    use_displaced_mesh = false
    variable = 'disp_x disp_y frictionless_normal_lm tangential_lm'
    boundary = '3'
    sort_by = id
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = false
  csv = false
  [console]
    type = Console
    max_rows = 5
  []
  [vectorpp_output]
    type = CSV
    create_final_symlink = true
    file_base = cylinder_friction
    execute_on = 'FINAL'
  []
[]

[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 2
    secondary_boundary = 3
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
  [weighted_gap_lm]
    type = ComputeFrictionalForceLMMechanicalContact
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    variable = frictionless_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    friction_lm = tangential_lm
    mu = 0.4
    c_t = 1.0e5
    c = 1.0e6
    weighted_gap_uo = weighted_vel_uo
    weighted_velocities_uo = weighted_vel_uo
  []
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
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
    primary_boundary = '2'
    secondary_boundary = '3'
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
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    variable = tangential_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    variable = tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
[]

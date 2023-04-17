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
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [penalty_normal_pressure]
    order = FIRST
    family = LAGRANGE
  []
  [penalty_frictional_pressure]
    order = FIRST
    family = LAGRANGE
  []
  [accumulated_slip_one]
    order = FIRST
    family = LAGRANGE
  []
  [tangential_vel_one]
    order = FIRST
    family = LAGRANGE
  []
  [weighted_gap]
    order = FIRST
    family = LAGRANGE
  []
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

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
    block = '1 2 3 4 5 6 7'
  []
[]

[AuxKernels]
  [penalty_normal_pressure_auxk]
    type = PenaltyMortarUserObjectAux
    variable = penalty_normal_pressure
    user_object = friction_uo
    contact_quantity = normal_pressure
  []
  [penalty_frictional_pressure_auxk]
    type = PenaltyMortarUserObjectAux
    variable = penalty_frictional_pressure
    user_object = friction_uo
    contact_quantity = tangential_pressure_one
  []
  [penalty_accumulated_slip_auxk]
    type = PenaltyMortarUserObjectAux
    variable = accumulated_slip_one
    user_object = friction_uo
    contact_quantity = accumulated_slip_one
  []
  [penalty_tangential_vel_auxk]
    type = PenaltyMortarUserObjectAux
    variable = tangential_vel_one
    user_object = friction_uo
    contact_quantity = tangential_velocity_one
  []
  [penalty_weighted_gap_auxk]
    type = PenaltyMortarUserObjectAux
    variable = weighted_gap
    user_object = friction_uo
    contact_quantity = weighted_gap
  []
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
    block = '1 2 3 4 5 6 7'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
    block = '1 2 3 4 5 6 7'
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
    block = '1 2 3 4 5 6 7'
  []
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
  [stuff1_strain]
    type = ComputeFiniteStrain
    block = '1'
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
  [stuff2_strain]
    type = ComputeFiniteStrain
    block = '2 3 4 5 6 7'
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-15          '
                        '         1e-5'

  line_search = 'none'

  nl_abs_tol = 1e-7

  start_time = 0.0
  end_time = 0.3 # 3.5
  l_tol = 1e-4
  dt = 0.1
  dtmin = 0.001
  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [x_disp]
    type = NodalValueSampler
    variable = disp_x
    boundary = '3'
    sort_by = id
  []
  [y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3'
    sort_by = id
  []
  [cont_press]
    type = NodalValueSampler
    variable = penalty_normal_pressure
    boundary = '3'
    sort_by = id
  []
  [friction]
    type = NodalValueSampler
    variable = penalty_frictional_pressure
    boundary = '3'
    sort_by = id
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = true
  csv = false
  [console]
    type = Console
    max_rows = 5
  []
  [chkfile]
    type = CSV
    show = 'x_disp y_disp cont_press friction'
    file_base = cylinder_friction_penalty_check
    create_final_symlink = true
    execute_on = 'FINAL'
  []
[]

[UserObjects]
  [friction_uo]
    type = PenaltyFrictionUserObject
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    disp_x = disp_x
    disp_y = disp_y
    friction_coefficient = 0.4 # with 2.0 works
    secondary_variable = disp_x
    penalty = 5e9
    penalty_friction = 2e7 # 1e7
  []
[]

[Constraints]
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
[]

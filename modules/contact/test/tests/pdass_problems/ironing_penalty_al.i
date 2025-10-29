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
  allow_renumbering = false
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
  [real_weighted_gap]
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
    x = '0. 8.'
    y = '0. 8.'
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
  [penalty_normal_pressure_auxk]
    type = MortarUserObjectAux
    variable = penalty_normal_pressure
    user_object = friction_uo
    contact_quantity = normal_pressure
  []
  [penalty_frictional_pressure_auxk]
    type = MortarUserObjectAux
    variable = penalty_frictional_pressure
    user_object = friction_uo
    contact_quantity = tangential_pressure_one
  []
  [penalty_accumulated_slip_auxk]
    type = MortarUserObjectAux
    variable = accumulated_slip_one
    user_object = friction_uo
    contact_quantity = accumulated_slip_one
  []
  [penalty_tangential_vel_auxk]
    type = MortarUserObjectAux
    variable = tangential_vel_one
    user_object = friction_uo
    contact_quantity = tangential_velocity_one
  []
  [real_weighted_gap_auxk]
    type = MortarUserObjectAux
    variable = real_weighted_gap
    user_object = friction_uo
    contact_quantity = normal_gap
  []
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

# [VectorPostprocessors]
#   [penalty_normal_pressure]
#     type = NodalValueSampler
#     variable = penalty_normal_pressure
#     boundary = 10
#     sort_by = id
#   []
# []

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [force_x]
    type = NodalSum
    boundary = 30
    variable = saved_x
  []
  [force_y]
    type = NodalSum
    boundary = 30
    variable = saved_y
  []
  [gap]
    type = SideExtremeValue
    value_type = min
    variable = real_weighted_gap
    boundary = 10
  []
  [num_al]
    type = NumAugmentedLagrangeIterations
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

  l_max_its = 7
  nl_max_its = 300

  start_time = 0.0
  end_time = 6.5

  dt = 0.0125
  dtmin = 1e-5
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

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = true
  csv = true
  # [chkfile]
  #   type = CSV
  #   start_time = 0.0
  #   execute_vector_postprocessors_on = FINAL
  # []

  [console]
    type = Console
    max_rows = 5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Problem]
  type = AugmentedLagrangianContactFEProblem
[]

[UserObjects]
  [friction_uo]
    type = PenaltyFrictionUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    disp_x = disp_x
    disp_y = disp_y
    friction_coefficient = 0.4 # with 2.0 works
    secondary_variable = disp_x
    penalty = 5e5
    penalty_friction = 1e4
    slip_tolerance = 1e-05
    penetration_tolerance = 1e-03
  []
[]

[Constraints]
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [t_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [t_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
[]

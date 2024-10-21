[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = cond_number.e
  []
  allow_renumbering = false
[]

[Problem]
  type = AugmentedLagrangianContactFEProblem
  extra_tag_vectors = 'ref'
  maximum_lagrangian_update_iterations = 1000
[]

[AuxVariables]
  [penalty_normal_pressure]
  []
  [penalty_frictional_pressure]
  []
  [accumulated_slip_one]
  []
  [tangential_vel_one]
  []
  [normal_gap]
  []
  [normal_lm]
  []
  [saved_x]
  []
  [saved_y]
  []
  [active]
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

[Physics/SolidMechanics/QuasiStatic/all]
  strain = FINITE
  add_variables = true
  save_in = 'saved_x saved_y'
  extra_vector_tags = 'ref'
  block = '1 2 3 4 5 6 7'
  generate_output = 'stress_xx stress_yy stress_xy'
[]

[AuxKernels]
  [penalty_normal_pressure]
    type = PenaltyMortarUserObjectAux
    variable = penalty_normal_pressure
    user_object = penalty_friction_object_al_friction
    contact_quantity = normal_pressure
    boundary = 3
  []
  [penalty_frictional_pressure]
    type = PenaltyMortarUserObjectAux
    variable = penalty_frictional_pressure
    user_object = penalty_friction_object_al_friction
    contact_quantity = tangential_pressure_one
    boundary = 3
  []
  [penalty_tangential_vel_one]
    type = PenaltyMortarUserObjectAux
    variable = tangential_vel_one
    user_object = penalty_friction_object_al_friction
    contact_quantity = tangential_velocity_one
    boundary = 3
  []
  [penalty_accumulated_slip_one]
    type = PenaltyMortarUserObjectAux
    variable = accumulated_slip_one
    user_object = penalty_friction_object_al_friction
    contact_quantity = accumulated_slip_one
    boundary = 3
  []
  [normal_lm]
    type = PenaltyMortarUserObjectAux
    variable = normal_lm
    user_object = penalty_friction_object_al_friction
    contact_quantity = normal_lm
    boundary = 3
  []
  [normal_gap]
    type = PenaltyMortarUserObjectAux
    variable = normal_gap
    user_object = penalty_friction_object_al_friction
    contact_quantity = normal_gap
    boundary = 3
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
  [_dt]
    type = TimestepSize
  []
  [num_lin_it]
    type = NumLinearIterations
  []
  [num_nonlin_it]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nonlin_it
  []
  [gap]
    type = SideExtremeValue
    value_type = min
    variable = normal_gap
    boundary = 3
  []
  [num_al]
    type = NumAugmentedLagrangeIterations
  []
  [active_set_size]
    type = NodalSum
    variable = active
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
    youngs_modulus = 1e8
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
  solve_type = 'NEWTON'

  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      8'

  line_search = 'none'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_max_its = 50
  l_tol = 1e-05
  l_abs_tol = 1e-13

  start_time = 0.0
  end_time = 0.2 # 1.0
  dt = 0.1
  dtmin = 0.1

  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []

  automatic_scaling = true
  compute_scaling_once = false
  off_diagonals_in_auto_scaling = true
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
    variable = 'disp_x disp_y penalty_normal_pressure penalty_frictional_pressure normal_gap'
    boundary = '3'
    sort_by = id
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = true
  csv = false
  [vectorpp_output]
    type = CSV
    create_final_symlink = true
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]

[Contact]
  [al_friction]
    formulation = mortar_penalty
    model = coulomb
    primary = '2'
    secondary = '3'
    penalty = 1e7
    penalty_friction = 1e+7
    friction_coefficient = 0.4
    al_penetration_tolerance = 1e-7
    al_incremental_slip_tolerance = 1.0 # Not active
    penalty_multiplier = 100
  []
[]

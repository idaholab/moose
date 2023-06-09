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

[AuxVariables]
  [penalty_normal_pressure]
  []
  [penalty_frictional_pressure]
  []
  [accumulated_slip_one]
  []
  [tangential_vel_one]
  []
  [weighted_gap]
  []
  [react_x]
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
  extra_vector_tags = 'ref'
  block = '1 2 3 4 5 6 7'
  generate_output = 'stress_xx stress_yy stress_xy'
[]

[AuxKernels]
  [penalty_normal_pressure]
    type = PenaltyMortarUserObjectAux
    variable = penalty_normal_pressure
    user_object = friction_uo
    contact_quantity = normal_pressure
  []
  [penalty_frictional_pressure]
    type = PenaltyMortarUserObjectAux
    variable = penalty_frictional_pressure
    user_object = friction_uo
    contact_quantity = tangential_pressure_one
  []
  [penalty_accumulated_slip]
    type = PenaltyMortarUserObjectAux
    variable = accumulated_slip_one
    user_object = friction_uo
    contact_quantity = accumulated_slip_one
  []
  [penalty_tangential_vel]
    type = PenaltyMortarUserObjectAux
    variable = tangential_vel_one
    user_object = friction_uo
    contact_quantity = tangential_velocity_one
  []
  [penalty_weighted_gap]
    type = PenaltyMortarUserObjectAux
    variable = weighted_gap
    user_object = friction_uo
    contact_quantity = weighted_gap
  []
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
  []
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
  []
[]

[Postprocessors]
  [bot_react_x]
    type = NodalSum
    variable = react_x
    boundary = 1
  []
  [bot_react_y]
    type = NodalSum
    variable = react_y
    boundary = 1
  []
  [top_react_x]
    type = NodalSum
    variable = react_x
    boundary = 4
  []
  [top_react_y]
    type = NodalSum
    variable = react_y
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
    variable = weighted_gap
    boundary = 3
  []
  [num_al]
    type = NumAugmentedLagrangeIterations
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
  [surface]
    type = NodalValueSampler
    use_displaced_mesh = false
    variable = 'disp_x disp_y penalty_normal_pressure penalty_frictional_pressure'
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
  [vectorpp_output]
    type = CSV
    create_final_symlink = true
    file_base = cylinder_friction_penalty
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
    penalty = 1e11
    penalty_friction = 5e8 # 1e7
  []
  [geo_surface]
    type = GeometrySphere
    boundary = 3
    center = '0 4 0'
    radius = 3
    execute_on = TIMESTEP_BEGIN
  []
  [geo_lower]
    type = GeometrySphere
    boundary = secondary_lower
    center = '0 4 0'
    radius = 3
    execute_on = TIMESTEP_BEGIN
  []
[]

[Adaptivity]
  [Markers]
    [contact_surface]
      type = BoundaryMarker
      mark = REFINE
      next_to = 3
    []
    [contact_lower]
      type = UniformMarker
      block = secondary_lower
      mark = REFINE
    []
    [contact]
      type = ComboMarker
      markers = 'contact_surface' # contact_lower
    []
  []
  initial_marker = contact
  initial_steps = 1
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

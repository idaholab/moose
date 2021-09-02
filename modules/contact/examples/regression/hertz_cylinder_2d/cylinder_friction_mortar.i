[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = cylinder.e
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
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [frictionless_normal_lm]
    order = FIRST
    family = LAGRANGE
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
  [diag_saved_x]
  []
  [diag_saved_y]
  []
[]

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. -0.025 -0.025'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. 0.0 0.01'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_xy vonmises_stress hydrostatic_stress'
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
    block = '1 2 3 4 5 6 7'
  []
[]

[AuxKernels]
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
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e10
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [tensor_cylinder]
    type = ComputeIsotropicElasticityTensor
    block = '2 3 4 5 6 7'
    youngs_modulus = 7000
    poissons_ratio = 0.3
  []
  [stress_cylinder]
    type = ComputeFiniteStrainElasticStress
    block = '2 3 4 5 6 7'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  # petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  # petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'
  snesmf_reuse_base = false
  nl_abs_tol = 1e-4
  nl_rel_tol = 1e-6
  l_max_its = 12
  nl_max_its = 50
  start_time = 0.0
  end_time = 0.25 # 3.5
  l_tol = 1e-6
  dt = 0.25
  dtmin = 0.001
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
    boundary = '3 4'
    sort_by = id
  []
  [y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3 4'
    sort_by = id
  []
  [cont_press]
    type = NodalValueSampler
    variable = frictionless_normal_lm
    boundary = '3'
    sort_by = x
  []
  [friction]
    type = NodalValueSampler
    variable = tangential_lm
    boundary = '3'
    sort_by = x
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = true
  csv = true
  [console]
    type = Console
    max_rows = 5
  []
  [chkfile]
    type = CSV
    show = 'x_disp y_disp cont_press friction'
    start_time = 0.0
    execute_vector_postprocessors_on = FINAL
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
    mu = 0.7
    # c_t = 1.0e15
    # c = 1.0e6
    c = 1.0e6
    c_t = 1.0e12
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
  []

[]

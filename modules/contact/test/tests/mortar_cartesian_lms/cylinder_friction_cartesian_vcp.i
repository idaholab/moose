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
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  converge_on = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [lm_x]
    block = 'secondary_lower'
    use_dual = true
    scaling = 1.0e-5
  []
  [lm_y]
    block = 'secondary_lower'
    use_dual = true
    scaling = 1.0e-5
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

[Modules/TensorMechanics/Master]
  [all]
    incremental = false
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
    block = '1 2 3 4 5 6 7'
    strain = SMALL
    add_variables = false
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
  [_dt]
    type = TimestepSize
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
  [stuff2_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2 3 4 5 6 7'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stuff1_stress]
    type = ComputeLinearElasticStress
    block = '1'
  []
  [stuff2_stress]
    type = ComputeLinearElasticStress
    block = '2 3 4 5 6 7'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'NONZERO               1e-12'

  line_search = 'none'
  nl_abs_tol = 1e-7
  l_max_its = 5
  nl_rel_tol = 1e-09
  start_time = -0.1
  end_time = 0.3 # 3.5
  l_tol = 1e-8
  dt = 0.1
  dtmin = 0.001
[]

[Preconditioning]
  [vcp]
    type = VCP
    full = true
    lm_variable = 'lm_x lm_y'
    primary_variable = 'disp_x disp_y'
    preconditioner = 'LU'
    is_lm_coupling_diagonal = false
    adaptive_condensation = true
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
  [lm_x]
    type = NodalValueSampler
    variable = lm_x
    boundary = '3'
    sort_by = id
  []
  [lm_y]
    type = NodalValueSampler
    variable = lm_y
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
    show = 'x_disp y_disp lm_x lm_y'
    file_base = cylinder_friction_check
    create_final_symlink = true
    execute_on = 'FINAL'
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeFrictionalForceCartesianLMMechanicalContact
    primary_boundary = 2
    secondary_boundary = 3
    primary_subdomain = 10000
    secondary_subdomain = 10001
    lm_x = lm_x
    lm_y = lm_y
    variable = lm_x
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    correct_edge_dropping = false
    mu = 0.4
    c_t = 1.0e6
    c = 1.0e6
  []
  [x]
    type = CartesianMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = lm_x
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = false
  []
  [y]
    type = CartesianMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = lm_y
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = false
  []

[]

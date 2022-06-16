[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = single_point_2d.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./penetration]
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./diag_saved_x]
  [../]
  [./diag_saved_y]
  [../]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
[]

[Functions]
  [./appl_disp]
    type = PiecewiseLinear
    x = '0 0.001  0.101'
    y = '0 0.0   -0.10'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
  [../]
[]

[AuxKernels]
  [./incslip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./incslip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  [../]
  [./accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = appl_disp
  [../]
  [./topy]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = -0.002001
  [../]
[]

[Materials]
  [./bot_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  [../]
  [./bot_strain]
    type = ComputePlaneFiniteStrain
    block = '1'
  [../]
  [./bot_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  [../]
  [./top_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./top_strain]
    type = ComputePlaneFiniteStrain
    block = '2'
  [../]
  [./top_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = NodalVariableValue
    nodeid = 5
    variable = disp_x
  [../]
  [./disp_y]
    type = NodalVariableValue
    nodeid = 5
    variable = disp_y
  [../]
  [./inc_slip_x]
    type = NodalVariableValue
    nodeid = 5
    variable = inc_slip_x
  [../]
  [./inc_slip_y]
    type = NodalVariableValue
    nodeid = 5
    variable = inc_slip_y
  [../]
  [./accum_slip_x]
    type = NodalVariableValue
    nodeid = 5
    variable = accum_slip_x
  [../]
  [./accum_slip_y]
    type = NodalVariableValue
    nodeid = 5
    variable = accum_slip_y
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -mat_superlu_dist_iterrefine'
  petsc_options_value = 'lu    superlu_dist 1'

  line_search = 'contact'
  contact_line_search_allowed_lambda_cuts = 0
  contact_line_search_ltol = 0.5

  l_max_its = 15
  nl_max_its = 10
  dt = 0.001
  end_time = 0.002
  num_steps = 10000
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  dtmin = 0.001
  l_tol = 1e-3
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  csv = true
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    primary = 2
    secondary = 3
    model = frictionless
    formulation = kinematic
    penalty = 1e12
    normalize_penalty = true
    tangential_tolerance = 1e-3
  [../]
[]

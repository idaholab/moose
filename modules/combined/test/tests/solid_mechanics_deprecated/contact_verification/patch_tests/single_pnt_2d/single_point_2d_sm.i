[Mesh]
  file = single_point_2d.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
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

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_x = saved_x
    save_in_disp_y = saved_y
    diag_save_in_disp_x = diag_saved_x
    diag_save_in_disp_y = diag_saved_y
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
  [./bottom]
    type = Elastic
    block = 1
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e9
    formulation = NonlinearPlaneStrain
  [../]
  [./top]
    type = Elastic
    block = 2
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    formulation = NonlinearPlaneStrain
  [../]
[]

[Postprocessors]
  [./bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  [../]
  [./bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  [../]
  [./top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 4
  [../]
  [./top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 4
  [../]
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

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu    superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 200
  dt = 0.001
  end_time = 0.001
  num_steps = 10000
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  dtmin = 0.001
  l_tol = 1e-3
[]

[Outputs]
  file_base = single_point_2d_out_glued_kin
  exodus = true
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
    disp_y = disp_y
    disp_x = disp_x
    model = glued
    formulation = kinematic
    penalty = 1e12
    normalize_penalty = true
    tangential_tolerance = 1e-3
  [../]
[]

[Mesh]
  file = 1x1x1_cube.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]

 []

[AuxVariables]
 [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 1'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
[]


[BCs]
  [./u_top_pull]
    type = Pressure
    variable = disp_y
    component = 1
    boundary = 5
    factor = -0.5e8
    function = top_pull
  [../]

  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]
[]

[Materials]
  [./elastic]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 2.e11
    poissons_ratio = .3
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK

  verbose = true
  print_linear_residuals = true
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-4
  l_tol           = 1e-5
  l_abs_step_tol  = 1e-6

  start_time = 0.0
  dtmin = 3.0
  end_time = 13.0

  [./TimeStepper]
    type = AdaptiveDT
    optimal_iterations = 1
    iteration_window = 1
    linear_iteration_ratio = 1
    dt = 5.0
  [../]

  restart_file_base = adapt_tstep_shrink_init_dt_restart1_out_restart_0001
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Output]
  file_base = adapt_tstep_shrink_init_dt_out
  interval = 1
  output_initial = true
  exodus = true
[]

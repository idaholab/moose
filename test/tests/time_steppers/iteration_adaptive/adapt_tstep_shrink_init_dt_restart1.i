[Mesh]
  file = 1x1x1_cube.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./func1]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 1'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bc1]
    type = FunctionDirichletBC
    variable = u
    boundary = 3
    function = func1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK

  verbose = true
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
  num_steps = 1

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 1
    iteration_window = 1
    linear_iteration_ratio = 1
    dt = 5.0
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Output]
  interval = 1
  output_initial = true
  linear_residuals = true
  exodus = true
  num_checkpoint_files = 1
[]

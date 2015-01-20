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
  [./knot]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0 0'
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
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-3
  l_tol = 1e-5

  start_time = 0.0
  end_time = 2.0
  timestep_tolerance = 0.3

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.9
    optimal_iterations = 10
    force_step_every_function_point = true
    max_function_change = 1e20
    timestep_limiting_function = knot
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
[]

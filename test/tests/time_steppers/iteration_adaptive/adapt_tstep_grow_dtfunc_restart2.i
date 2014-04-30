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
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-3
  l_tol = 1e-5

  start_time = 0.0
  end_time = 20.0
  num_steps = 5

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
    optimal_iterations = 10
    time_t  = '0.0, 5.0'
    time_dt = '1.0, 5.0'
  [../]

  restart_file_base = adapt_tstep_grow_dtfunc_restart1_out_cp/0004
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  file_base = adapt_tstep_grow_dtfunc_out_2
  output_initial = true
  exodus = true
  [./console]
    type = Console
    linear_residuals = true
    perf_log = true
  [../]
[]

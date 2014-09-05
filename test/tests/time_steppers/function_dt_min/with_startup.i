# Just tests the startup capability for FunctionDT.
[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 1
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./dudt]
    type = TimeDerivative
    variable = u
  [../]
  [./body]
    type = BodyForce
    variable = u
    value = -0.01
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  end_time = 100
  start_time = -2
  n_startup_steps = 1

  [./TimeStepper]
    type = FunctionDT
    time_t = '0  10 50 90 100'
    time_dt ='2  5  5  2   3'
    min_dt = 2
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
  [./console]
    type = Console
    linear_residuals = true
    all_variable_norms = true
    max_rows = 200
  [../]
[]
